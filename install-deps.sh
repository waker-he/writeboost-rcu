#!/bin/sh

sudo apt update
sudo apt install gcc
sudo apt install cmake
sudo apt install python3
sudo apt install python3-pip
sudo pip install conan
conan profile detect --force