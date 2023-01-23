#!/bin/sh

mkdir -m 755 -p /usr/share/ibus/component
mkdir -m 755 -p /usr/lib/ibus

ln -s $(realpath dank-emoji-picker-ibus.xml) /usr/share/ibus/component/dank-emoji-picker-ibus.xml
ln -s $(realpath build/dank-emoji-picker-ibus) /usr/lib/ibus/dank-emoji-picker-ibus
ibus write-cache
