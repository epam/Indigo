FROM centos:7

RUN yum clean -y all
RUN yum install -y epel-release centos-release-scl
RUN yum update -y
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-SIG-SCLo
RUN yum install -y git cmake3 devtoolset-9-gcc-c++ make freetype-devel fontconfig-devel git-lfs
RUN ln -fs /usr/bin/cmake3 /usr/local/bin/cmake && \
    ln -fs /usr/bin/cpack3 /usr/local/bin/cpack && \
	ln -fs /usr/bin/ctest3 /usr/local/bin/ctest && \
RUN yum clean -y all

CMD ["scl", "enable", "devtoolset-9", "bash"]

RUN source scl_source enable devtoolset-9 && \
    make --version && \
    cmake --version && \
    git --version && \
    gcc --version && \
    g++ --version && \
    python --version
