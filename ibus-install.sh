#!/bin/sh

mkdir -p /usr/share/ibus/component
mkdir -p /usr/lib/ibus

ln -s $(realpath dank-emoji-picker-ibus.xml) /usr/share/ibus/component
ln -s $(realpath build/dank-emoji-picker-ibus) /usr/lib/ibus
ibus write-cache
