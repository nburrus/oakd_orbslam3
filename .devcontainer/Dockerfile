# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.238.0/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version (use Debian 11, Ubuntu 18.04/22.04 on local arm64/Apple Silicon): debian-11, debian-10, ubuntu-22.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="bullseye"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Install CMake version different from what base image has already installed. 
# CMake reinstall choices: none, 3.21.5, 3.22.2, or versions from https://cmake.org/download/
ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/
RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y -qq update && apt-get -y install --no-install-recommends \
    wget \
    cmake-curses-gui \
    ninja-build \
    libusb-1.0-0-dev \
    libgl-dev \
    libglew-dev \
    libboost-dev \
    libssl-dev

# OpenCV 
COPY install_opencv.sh /tmp
RUN /bin/bash /tmp/install_opencv.sh

# More for ORB-SLAM
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    libboost-serialization-dev \
    libilmbase-dev \
    libopenexr-dev

# For depth-ai. Not sure which ones are actually required though.
RUN export DEBIAN_FRONTEND=noninteractive && apt-get -y -qq update \
    && apt-get install -y wget build-essential libjpeg-dev libtiff5-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libgtk2.0-dev libgtk-3-dev libatlas-base-dev gfortran

# Depth-ai usb rules. Probably not necessary in the container.
RUN export DEBIAN_FRONTEND=noninteractive && apt-get -y -qq update && apt-get install -y udev
RUN echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="03e7", MODE="0666"' | tee /etc/udev/rules.d/80-movidius.rules
