#!/bin/sh

ln -s $(realpath rime.xml) /usr/share/ibus/component/
ln -s $(realpath build/ibus-test) /usr/local/bin/
ibus write-cache
