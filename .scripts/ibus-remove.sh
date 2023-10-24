#!/bin/sh

rm -f /usr/share/ibus/component/ibusimemojipicker.xml

rm -f /usr/lib/ibus/ibusimemojipicker

rm -f /usr/share/icons/hicolor/32x32/apps/im-emoji-picker.png
touch /usr/share/icons/hicolor

ibus write-cache
