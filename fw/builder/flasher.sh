#!/bin/bash
#
#  Standalone flash utility for unified images.
#
#  Ex.  ./flasher <unified_hexfile.hex path>
#
echo $'device nrf51822\n'\
     $'speed 1000\n'\
     $'w4 4001e504 2\n'\
     $'w4 4001e50c 1\n'\
     $'sleep 100\n'\
     $'r\n'\
     $'loadbin ' $1 $' 0\n'\
     $'r\n'\
     $'g\n'\
     $'exit\n'\
     > /tmp/unified_flash.jlink

JLinkExe -if SWD /tmp/unified_flash.jlink