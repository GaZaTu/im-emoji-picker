#!/bin/sh

__DIRNAME=$(realpath $(dirname "$0"))

mkdir -m 755 -p /usr/share/ibus/component
ln -s $(realpath $__DIRNAME/../ibus-component.xml) /usr/share/ibus/component/im-emoji-picker-ibus.xml

mkdir -m 755 -p /usr/lib/ibus
ln -s $(realpath $__DIRNAME/../build/im-emoji-picker-ibus) /usr/lib/ibus/im-emoji-picker-ibus

mkdir -m 755 -p /usr/share/icons/hicolor/32x32/apps
ln -s $(realpath $__DIRNAME/../src/res/im-emoji-picker_32x32.png) /usr/share/icons/hicolor/32x32/apps/im-emoji-picker.png
touch /usr/share/icons/hicolor

ibus write-cache