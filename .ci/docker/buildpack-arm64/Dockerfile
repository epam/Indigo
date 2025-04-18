FROM ubuntu:20.04
RUN apt update && \
    apt upgrade -y && \
    apt install -y --no-install-recommends \
        apt-transport-https \
        curl \
        software-properties-common \
        gpg-agent \
        lsb-release \
        && \
    export UBUNTU_VERSION=$(lsb_release -cs) && \
    # CMake
    curl -sL https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add && \
    add-apt-repository "deb https://apt.kitware.com/ubuntu/ ${UBUNTU_VERSION} main" -y && \
    # Set up ARM64 repositories
    sed -i 's/deb/deb [arch=amd64]/g' /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ ${UBUNTU_VERSION} main restricted" >> /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ ${UBUNTU_VERSION}-updates main restricted" >> /etc/apt/sources.list && \
    dpkg --add-architecture arm64 && \
    apt update && \
    # Install tools required for building Indigo
    apt install -y --no-install-recommends \
        cmake=3.31.\* cmake-data=3.31.\* \
        g++-10-aarch64-linux-gnu \
        git \
        libfontconfig1-dev:arm64 \
        libfreetype6-dev:arm64 \
        make \
        qemu-user-static

ENV CC=aarch64-linux-gnu-gcc-10
ENV CXX=aarch64-linux-gnu-g++-10
ENV INDIGO_QEMU_BINARY=qemu-aarch64-static
ENV ASAN_OPTIONS=alloc_dealloc_mismatch=0
RUN echo $'leak:libfontconfig\nleak:libc\n' >> /opt/external.supp
ENV LSAN_OPTIONS=suppressions=/opt/external.supp

RUN cmake --version && \
    aarch64-linux-gnu-gcc-10 --version && \
    aarch64-linux-gnu-g++-10 --version && \
    git --version && \
    make --version

# NOTE: This is for testing purposes
# RUN git clone https://github.com/epam/indigo && \
#     cd indigo && \
#     mkdir build && \
#     cd build && \
#     cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_BINGO=OFF -DBUILD_BINGO_ELASTIC=OFF -DBUILD_INDIGO_WRAPPERS=OFF -DBUILD_INDIGO_UTILS=OFF -DCMAKE_SYSTEM_PROCESSOR_LOWER=aarch64 && \
#     cmake --build . --config Release -- -j $(nproc) && \
#     ctest --verbose
