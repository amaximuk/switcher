#!/bin/bash

cd build
cmake -DQt5_DIR=/mnt/data/3rdparty/qt/5.15.2/gcc_64/lib/cmake/Qt5 -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ..
cd ..
