#!/bin/bash
#****************************************************************************
#*
#*  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
#*
#*  This code is licensed under the MIT.
#*
#****************************************************************************

dd if=/dev/urandom of=iv.bin bs=1 count=16
dd if=/dev/urandom of=key.bin bs=1 count=16
