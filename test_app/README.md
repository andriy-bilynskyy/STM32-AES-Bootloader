# Bootloader test application

## Just a stupid application to demonstrate bootloader functionality

The application is blinking LED using about 1S period.

## Important!

Bootloader application during compilation estimates its size.
So application flash section is dynamically calculated.
*If you made any changes in bootloader execute bootloader and issue command **info** in bootloader console.*

	info
	Boot Loader: 0x8000000 - 0x8004b33 (19252 bytes)
	Application: 0x8004c00 - 0x800ffff (46080 bytes)
	Page size: 1024 bytes

*It shows the application address and its size. In sample **Application: 0x8004c00 - 0x800ffff (46080 bytes)**
Based on this data application linker script should be updated. So open file **stm32_app.ld** and change FLASH area according to this data.*

	FLASH (rx)      : ORIGIN = 0x08004c00, LENGTH = 45K

Actually its only one difference from usual (without bootloader) STM32 application. :( Some FLASH space is reserved by bootloader.
