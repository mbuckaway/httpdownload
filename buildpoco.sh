#!/bin/sh
pushd $(pwd)
git submodule init
git submodule update
cd poco
# Turn off Apache connector
bash build_cmake.sh -DPOCO_ENABLE_APACHECONNECTOR=OFF -DPOCO_VERBOSE_MESSAGES=ON -DCMAKE_INSTALL_PREFIX=$(pwd)
popd

