#!/bin/sh
CDW=$(pwd)
pushd $(CWD)
if [ -d poco ]
then
    git submodule https://github.com/pocoproject/poco.git poco
fi
cd poco
# Turn off Apache connector
bash build_cmake.sh -DPOCO_ENABLE_APACHECONNECTOR=OFF -DPOCO_VERBOSE_MESSAGES=ON -DCMAKE_INSTALL_PREFIX=$(pwd)
popd

