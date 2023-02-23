#!/bin/sh

__DIRNAME=$(realpath $(dirname "$0"))

mkdir -m 755 -p /usr/share/fcitx5/addon
ln -s $(realpath $__DIRNAME/../fcitx5-addon.conf) /usr/share/fcitx5/addon/fcitx5imemojipicker.conf

mkdir -m 755 -p /usr/lib/fcitx5
ln -s $(realpath $__DIRNAME/../build/fcitx5imemojipicker.so) /usr/lib/fcitx5/fcitx5imemojipicker.so

mkdir -m 755 -p /usr/share/icons/hicolor/32x32/apps
ln -s $(realpath $__DIRNAME/../src/res/im-emoji-picker_32x32.png) /usr/share/icons/hicolor/32x32/apps/im-emoji-picker.png
touch /usr/share/icons/hicolor
