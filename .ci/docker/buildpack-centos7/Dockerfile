FROM centos:7

RUN yum install -y epel-release centos-release-scl
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-SIG-SCLo
RUN yum update -y
RUN yum install -y cmake3 devtoolset-9-gcc-c++ freetype-devel fontconfig-devel make rh-git218 rh-git218-git-lfs
RUN ln -fs /usr/bin/cmake3 /usr/local/bin/cmake && \
    ln -fs /usr/bin/cpack3 /usr/local/bin/cpack && \
	ln -fs /usr/bin/ctest3 /usr/local/bin/ctest
RUN yum clean -y all

# Enable the SCL for all bash scripts.
ENV MANPATH=/opt/rh/rh-git218/root/usr/share/man:/opt/rh/devtoolset-9/root/usr/share/man \
    X_SCLS="devtoolset-9 rh-git218" \
    PCP_DIR=/opt/rh/devtoolset-9/root \
    PATH=/opt/rh/rh-git218/root/usr/bin:/opt/rh/devtoolset-9/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    PKG_CONFIG_PATH=/opt/rh/devtoolset-9/root/usr/lib64/pkgconfig \
    LD_LIBRARY_PATH=/opt/rh/httpd24/root/usr/lib64:/opt/rh/devtoolset-9/root/usr/lib64:/opt/rh/devtoolset-9/root/usr/lib:/opt/rh/devtoolset-9/root/usr/ib64/dyninst:/opt/rh/devtoolset-9/root/usr/lib/dyninst:/opt/rh/devtoolset-9/root/usr/lib64:/opt/rh/devtoolset-9/root/usr/lib

RUN cmake --version && \
    gcc --version && \
    g++ --version && \
    git --version && \
    git-lfs --version && \
	make --version && \
    python --version
