#!/bin/bash

OPENCV_VERSION=4.5.5

sudo apt-get -y update && apt-get -qq install -y --no-install-recommends \
        build-essential cmake \
        wget unzip \
        libopenblas0 libopenblas-dev \
        libjpeg8 libjpeg8-dev \
        libpng16-16 libpng-dev \
        libtiff5 libtiff-dev \
        libwebp6 libwebp-dev \
        libopenjp2-7 libopenjp2-7-dev \
        libtbb2 libtbb-dev \
        libeigen3-dev \
        libgtk2.0-dev \
        python3 python3-pip python3-numpy python3-dev

cd /tmp
wget -q --no-check-certificate https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip -O opencv.zip
wget -q --no-check-certificate https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip -O opencv_contrib.zip
unzip -qq opencv.zip -d /tmp && rm -rf opencv.zip
unzip -qq opencv_contrib.zip -d /tmp && rm -rf opencv_contrib.zip

mkdir opencv-build && cd opencv-build
cmake \
    -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D OPENCV_EXTRA_MODULES_PATH=/tmp/opencv_contrib-${OPENCV_VERSION}/modules \
    -D EIGEN_INCLUDE_PATH=/usr/include/eigen3 \
    -D OPENCV_ENABLE_NONFREE=ON \
    -D WITH_JPEG=ON \
    -D WITH_PNG=ON \
    -D WITH_TIFF=ON \
    -D WITH_WEBP=ON \
    -D WITH_JASPER=ON \
    -D WITH_EIGEN=ON \
    -D WITH_TBB=ON \
    -D WITH_LAPACK=ON \
    -D WITH_PROTOBUF=ON \
    -D WITH_V4L=OFF \
    -D WITH_GSTREAMER=OFF \
    -D WITH_GTK=ON \
    -D WITH_QT=OFF \
    -D WITH_CUDA=OFF \
    -D WITH_VTK=OFF \
    -D WITH_OPENEXR=OFF \
    -D WITH_FFMPEG=OFF \
    -D WITH_OPENCL=OFF \
    -D WITH_OPENNI=OFF \
    -D WITH_XINE=OFF \
    -D WITH_GDAL=OFF \
    -D WITH_IPP=OFF \
    -D BUILD_OPENCV_PYTHON3=ON \
    -D BUILD_OPENCV_PYTHON2=OFF \
    -D BUILD_OPENCV_JAVA=OFF \
    -D BUILD_TESTS=OFF \
    -D BUILD_IPP_IW=OFF \
    -D BUILD_PERF_TESTS=OFF \
    -D BUILD_EXAMPLES=OFF \
    -D BUILD_ANDROID_EXAMPLES=OFF \
    -D BUILD_DOCS=OFF \
    -D BUILD_ITT=OFF \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D INSTALL_C_EXAMPLES=OFF \
    -D INSTALL_TESTS=OFF \
    /tmp/opencv-${OPENCV_VERSION}

make -j$(nproc)
sudo make install
rm -rf /tmp/opencv-build/*
rm -rf /tmp/opencv-${OPENCV_VERSION}
rm -rf /tmp/opencv_contrib-${OPENCV_VERSION}

