#!/bin/sh

sudo apt install cmake
sudo apt install clang-format

sudo apt install python3
sudo apt install python3-pip
sudo pip install conan
conan profile detect --force