#!/bin/bash

set -e

cd /workspaces/oakd_orbslam3
cd deps/pangolin
mkdir -p build && cd build
cmake .. -GNinja
ninja
sudo ninja install

cd /workspaces/oakd_orbslam3/ORB_SLAM3
./build.sh

# Sample run on TUM-VI
# Examples/Stereo-Inertial/stereo_inertial_tum_vi Vocabulary/ORBvoc.txt Examples/Stereo-Inertial/TUM-VI.yaml dataset-room1_512_16/mav0/cam0/data dataset-room1_512_16/mav0/cam1/data dataset-room1_512_16/dso/cam0/times.txt dataset-room1_512_16/mav0/imu0/data.csv

# Not sure why I need that everytime, but otherwise it fails to find some
# pangolin libraries in /usr/local/lib
sudo ldconfig
