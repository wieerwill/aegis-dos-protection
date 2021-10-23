#!/bin/bash
###########################################################
## AEGIS INSTALLATION
## This script will help you install AEGIS on your system
## It will build meson with buildtype release
## and optimization level 3.
## Run it inside your terminal in the root folder of the
## project.
###########################################################

# check user privilege
if [ `whoami` != 'root' ]; then
    echo "Please run this as root..., don't worry about it..."
    exit 1
fi

rm -r build_release
meson --buildtyp release -Db_lto = true --optimization 3 build_release
echo "########\nmeson finished preparations"
cd build_release
ninja
echo "########\nninja finished local build"
strip aegis
cd ..
meson install
echo "########\nAEGIS is now installed on your system"
