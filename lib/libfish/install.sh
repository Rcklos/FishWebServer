#!/bin/bash

mkdir -p build
cd build
cmake ../src && make -j4 && make install
ldconfig
echo "install cofish completed!!!!"
