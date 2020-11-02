FROM centos:7

RUN yum install -y epel-release centos-release-scl
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-SIG-SCLo
RUN yum update -y
RUN yum install -y cmake3 devtoolset-9-gcc-c++ freetype-devel fontconfig-devel make rh-git218
RUN ln -fs /usr/bin/cmake3 /usr/local/bin/cmake && \
    ln -fs /usr/bin/cpack3 /usr/local/bin/cpack && \
	ln -fs /usr/bin/ctest3 /usr/local/bin/ctest
RUN yum clean -y all
RUN source scl_source enable devtoolset-9 rh-git218 && \    
    cmake --version && \
    gcc --version && \
    g++ --version && \
    git --version && \
	make --version && \
    python --version

ENTRYPOINT ["scl", "enable", "devtoolset-9", "rh-git218", "bash"]
