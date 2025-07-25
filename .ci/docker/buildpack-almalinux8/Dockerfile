FROM almalinux:8

ARG DEVTOOLSET_VERSION=11

RUN yum install -y yum-utils cmake gcc gcc-c++ libstdc++ python3.12 python3.12-pip git
RUN alternatives --set python /usr/bin/python3.12
RUN alternatives --set python3 /usr/bin/python3.12
RUN yum update -y
RUN yum install -y gcc-toolset-${DEVTOOLSET_VERSION}-gcc-c++ gcc-toolset-${DEVTOOLSET_VERSION}-libasan-devel gcc-toolset-${DEVTOOLSET_VERSION}-liblsan-devel freetype-devel fontconfig-devel make python3
RUN yum clean -y all
RUN echo $'leak:libfontconfig\nleak:libc\n' >> /opt/external.supp
RUN git config --global --add safe.directory /github/workspace

# Enable the SCL for all bash scripts.
ENV MANPATH=/opt/rh/rh-git${GIT_VERSION}/root/usr/share/man:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/share/man \
    X_SCLS="gcc-toolset-${DEVTOOLSET_VERSION} rh-git${GIT_VERSION}" \
    PCP_DIR=/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root \
    PATH=/opt/rh/rh-git${GIT_VERSION}/root/usr/bin:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    PKG_CONFIG_PATH=/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64/pkgconfig \
    LD_LIBRARY_PATH=/opt/rh/httpd24/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64/dyninst:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib/dyninst:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib \
    LSAN_OPTIONS=suppressions=/opt/external.supp \
    ASAN_OPTIONS=alloc_dealloc_mismatch=0

RUN cmake --version && \
    gcc --version && \
    g++ --version && \
    git --version && \
    make --version && \
    python --version && \
    python3 --version
