#!/bin/sh

rm -f /usr/share/ibus/component/im-emoji-picker-ibus.xml

rm -f /usr/lib/ibus/im-emoji-picker-ibus

rm -f /usr/share/icons/hicolor/32x32/apps/im-emoji-picker.png
touch /usr/share/icons/hicolor

ibus write-cache
