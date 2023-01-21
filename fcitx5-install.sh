#!/bin/sh

mkdir -p /usr/share/fcitx5/addon
mkdir -p /usr/share/fcitx5/inputmethod
mkdir -p /usr/lib/fcitx5

ln -s $(realpath dank-emoji-picker-fcitx5-addon.conf) /usr/share/fcitx5/addon/fcitx5emoji.conf
ln -s $(realpath dank-emoji-picker-fcitx5.conf) /usr/share/fcitx5/inputmethod/fcitx5emoji.conf
ln -s $(realpath build/dank-emoji-picker-fcitx5.so) /usr/lib/fcitx5/fcitx5emoji.so
