# STM32 AES Bootloader

## Software description

That's very easy to protect commercial embedded software from copying - just set flash lock bit on your MCU. But there is open question during maintain: How to deliver software updates caused by implementing new features or bug fixing? In that case Bootloader is used, it's small application which is started after MCU start and somehow able to update flash with new main application and start running application.

Actually at customer side exists 2 problem:

- SW/HW equipment to make flash procedure
- Show application to customer (flash lock was initially used)

That software is solving both issues.

- In case of flashing HW/SW is used internal MCU USB port which provides CDC (virtual serial port). In that case customer just needs USB cable and standard serial port terminal software for example minicom.

- Flash .hex file is encrypted before delivery. Bootloader knows how to decrypt it and flash into MCU. (On MCU flash lock is used so for customer there is no access to bootloader code and decrypted application code).

To encrypt application .hex file the **AES-128** algorithm is used in **CTR mode**. Encrypted file seems as valid .hex file but all memory content is encrypted so it's possible to flash it but it will do nothing.

All software are compilable under Linux, see https://github.com/andriy-bilynskyy/STM32-Linux for details.

For debug prints semihosting is used (SEGGER RTT seems better but bootloader is used for commercial software in that case I avoid using non fully free software.)

For demonstration cheap **Blue-Pill** STM32 board is used https://ru.aliexpress.com/item/STM32F103C8T6-ARM-STM32-arduino/32887666464.html?spm=2114.13010708.0.0.134f33edtKnEcc. The software can be adopted to use another hardware (Just configuration for another stm32f1x MCUs, replacing HAL for another MCUs).

## Software components

- STM32F10x standard peripheral library. https://www.st.com/en/embedded-software/stsw-stm32054.html

- STM32F10x, STM32L1xx and STM32F3xx USB full speed device library.  https://www.st.com/en/embedded-software/stsw-stm32121.html (Also includes standard peripheral library.)

- tiny-AES-c. https://github.com/kokke/tiny-AES-c

- FreeRTOS+CLI https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_Command_Line_Interface.html (Used only cli files. To make it compilable created FreeRTOS stubs.)

## How to use

Update AES secret (for testing you can use existing)

	./keys/keygen.sh

Compile bootloder

	make clean build

Connect your board using j-tag and flash bootloder

	make erase
	make flash

Disconnect j-tag and power cycle your board (!that's very important! Flash protection is set at this stage. STM has strange behavior when protection is set with inserted j-tag.)

Enter bootloader: set BOOT1 pin to "1" (BOOT0 as usual to "0"), reset MCU, attach USB cable. Green LED on board should switch on which indicates CDC connection. On PC in /dev folder should appear new serial device **ttyACM0** (can be another in your case).

Open serial terminal and check available commands by issue **help** command.

	minicom --device /dev/ttyACM0

In terninal:

	help

	help:
	 Lists all the registered commands

	info:
	 Show chip info

	erase:
	 Erase application

	program
	 Program application

	status
	 Program status

	info
	Boot Loader: 0x8000000 - 0x8004b33 (19252 bytes)
	Application: 0x8004c00 - 0x800ffff (46080 bytes)
	Page size: 1024 bytes

*Exit terminal and start with test application downloading*

Compile test application

	cd test_app/
	make clean build

Encrypt application

	cd ../encrypter/
	make clean build run

As result you have encrypted application **test_app/stm32-app.hex.enc** path from project root.

Start serial terminal again and issue command **program**

	program
	---------------------------------------------
	Now send your .hex file as ASCII.

Send encrypted file (Ctrl+A S for minicom).

When file sent check flashing status by command **status**

	status
	Program OK!

Run application: set BOOT1 pin to "0" (BOOT0 as usual to "0"), reset MCU. Green LED on board should start blinking as described in simple test application.
