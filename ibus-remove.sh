#!/bin/sh

rm -f /usr/share/ibus/component/dank-emoji-picker-ibus.xml
rm -f /usr/local/bin/dank-emoji-picker-ibus
ibus write-cache
