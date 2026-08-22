# Build the exact Bingo PostgreSQL library from the checked-out source tree.
# Keep the PostgreSQL development toolchain isolated under /usr/pgsql-17 so
# CMake cannot accidentally select headers from another installed PG major.
FROM almalinux:8 AS bingo-builder

ARG DEVTOOLSET_VERSION=11
ARG POSTGRES_MAJOR_VERSION=17

USER root

RUN yum install -y \
      yum-utils \
      cmake \
      gcc \
      gcc-c++ \
      libstdc++ \
      python3.12 \
      python3.12-pip \
      git \
      curl \
    && alternatives --set python /usr/bin/python3.12 \
    && alternatives --set python3 /usr/bin/python3.12 \
    && yum update -y \
    && yum install -y \
      gcc-toolset-${DEVTOOLSET_VERSION}-gcc-c++ \
      gcc-toolset-${DEVTOOLSET_VERSION}-libasan-devel \
      gcc-toolset-${DEVTOOLSET_VERSION}-liblsan-devel \
      freetype-devel \
      fontconfig-devel \
      make \
      python3 \
    && yum clean -y all

RUN printf 'leak:libfontconfig\nleak:libc\n' > /opt/external.supp

# Enable the GCC toolset for every subsequent RUN command. This mirrors the
# maintained AlmaLinux 8 CI buildpack and the PostgreSQL 17 builder that has
# previously been used successfully for Bingo.
ENV PCP_DIR=/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root \
    PATH=/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    PKG_CONFIG_PATH=/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64/pkgconfig \
    LD_LIBRARY_PATH=/opt/rh/httpd24/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64/dyninst:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib/dyninst:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib64:/opt/rh/gcc-toolset-${DEVTOOLSET_VERSION}/root/usr/lib \
    LSAN_OPTIONS=suppressions=/opt/external.supp \
    ASAN_OPTIONS=alloc_dealloc_mismatch=0

RUN cmake --version \
    && gcc --version \
    && g++ --version \
    && git --version \
    && make --version \
    && python3 --version

WORKDIR /src
COPY . /src

# Install the complete PostgreSQL 17 client/development dependency set through
# PGDG's package manager rather than installing the -devel RPM with --nodeps.
# The latter leaves required companion packages (including pg_config) absent.
# Disable AlmaLinux's PostgreSQL module so it cannot interfere with the PGDG
# versioned packages or introduce a different major into discovery.
RUN set -eux; \
    yum install -y https://download.postgresql.org/pub/repos/yum/reporpms/EL-8-x86_64/pgdg-redhat-repo-latest.noarch.rpm; \
    yum -qy module disable postgresql || true; \
    yum install -y \
      postgresql${POSTGRES_MAJOR_VERSION} \
      postgresql${POSTGRES_MAJOR_VERSION}-devel; \
    rpm -q \
      postgresql${POSTGRES_MAJOR_VERSION} \
      postgresql${POSTGRES_MAJOR_VERSION}-libs \
      postgresql${POSTGRES_MAJOR_VERSION}-devel; \
    test -x /usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config; \
    /usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --version | grep -E '^PostgreSQL 17([.]|$)'; \
    /usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --includedir; \
    /usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --includedir-server; \
    /usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --libdir; \
    test -f "$(/usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --includedir-server)/postgres.h"; \
    test -f "$(/usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin/pg_config --libdir)/libpq.so"; \
    yum clean -y all

ENV PATH=/usr/pgsql-${POSTGRES_MAJOR_VERSION}/bin:${PATH}

# Bingo derives version metadata from Git. Mark the copied checkout safe so a
# container build cannot fall back to the bogus 1.9.1-00000000 metadata caused
# by Git's dubious-ownership protection.
RUN git config --global --add safe.directory /src \
    && test -d /src/.git \
    && git rev-parse HEAD > /tmp/source-revision \
    && (git describe --tags --always --dirty > /tmp/source-describe || git rev-parse --short HEAD > /tmp/source-describe)

# Pin FindPostgreSQL's root and cache variables to PGDG's versioned PostgreSQL
# 17 tree. FindPostgreSQL explicitly honors these cache entries, so an unrelated
# /usr/include/postgresql/<major> installation cannot win discovery.
RUN cmake -S /src -B /build-pg17 \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_BINGO_POSTGRES=ON \
      -DBUILD_INDIGO=OFF \
      -DBUILD_INDIGO_WRAPPERS=OFF \
      -DBUILD_BINGO_SQLSERVER=OFF \
      -DBUILD_BINGO_ORACLE=OFF \
      -DBUILD_BINGO_ELASTIC=OFF \
      -DBUILD_INDIGO_UTILS=OFF \
      -DCMAKE_PREFIX_PATH=/usr/pgsql-${POSTGRES_MAJOR_VERSION} \
      -DPostgreSQL_ROOT=/usr/pgsql-${POSTGRES_MAJOR_VERSION} \
      -DPostgreSQL_INCLUDE_DIR=/usr/pgsql-${POSTGRES_MAJOR_VERSION}/include \
      -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/pgsql-${POSTGRES_MAJOR_VERSION}/include/server \
      -DPostgreSQL_LIBRARY=/usr/pgsql-${POSTGRES_MAJOR_VERSION}/lib/libpq.so \
    && grep -E '^PostgreSQL_(INCLUDE_DIR|TYPE_INCLUDE_DIR|LIBRARY|VERSION)' /build-pg17/CMakeCache.txt \
    && cmake --build /build-pg17 \
      --config Release \
      --target package-bingo-postgres \
      -- -j "$(nproc)"

# Use Indigo's generated PostgreSQL package as the handoff between stages. This
# verifies both the library build and the same packaging path used for releases.
RUN set -eux; \
    package_dir="$(find /build-pg17 -maxdepth 1 -type d -name 'bingo-postgres17-*' -print -quit)"; \
    test -n "$package_dir"; \
    test -s "$package_dir/lib/libbingo-postgres.so"; \
    cd "$package_dir"; \
    sh ./bingo-pg-install.sh -pglibdir -y; \
    test -s bingo_install.sql; \
    mkdir -p /out; \
    cp lib/libbingo-postgres.so /out/libbingo-postgres.so; \
    cp bingo_install.sql /out/bingo_install.sql; \
    cp /tmp/source-revision /out/source-revision; \
    cp /tmp/source-describe /out/source-describe; \
    sha256sum /out/libbingo-postgres.so > /out/libbingo-postgres.sha256


# Runtime image used by every harness service: normal primary, crash instance,
# physical standby, and the test runners.
FROM postgres:17-bookworm AS postgres-bingo

USER root

COPY --from=bingo-builder /out/libbingo-postgres.so /usr/lib/postgresql/17/lib/libbingo-postgres.so
COPY --from=bingo-builder /out/bingo_install.sql /docker-entrypoint-initdb.d/00-bingo-install.sql
COPY --from=bingo-builder /out/libbingo-postgres.sha256 /opt/bingo-harness/libbingo-postgres.sha256
COPY --from=bingo-builder /out/source-revision /opt/bingo-harness/source-revision
COPY --from=bingo-builder /out/source-describe /opt/bingo-harness/source-describe

# Building on AlmaLinux 8 and running on Debian Bookworm is intentional: the
# older glibc build environment is forward-compatible with Bookworm. ldd makes
# any missing runtime dependency fail during image construction instead of at
# PostgreSQL startup.
RUN test -s /usr/lib/postgresql/17/lib/libbingo-postgres.so \
    && ldd /usr/lib/postgresql/17/lib/libbingo-postgres.so \
    && ! ldd /usr/lib/postgresql/17/lib/libbingo-postgres.so | grep -q 'not found'

COPY bingo/postgres/tests/docker/init-primary-replication.sh /docker-entrypoint-initdb.d/10-replication.sh
COPY bingo/postgres/tests/docker/standby-entrypoint.sh /opt/bingo-harness/standby-entrypoint.sh
COPY bingo/postgres/tests/docker/crash-entrypoint.sh /opt/bingo-harness/crash-entrypoint.sh
COPY bingo/postgres/tests/docker/run-suite.sh /opt/bingo-harness/run-suite.sh
COPY bingo/postgres/tests/local/ /opt/bingo-tests/

RUN chmod 0755 \
      /docker-entrypoint-initdb.d/10-replication.sh \
      /opt/bingo-harness/standby-entrypoint.sh \
      /opt/bingo-harness/crash-entrypoint.sh \
      /opt/bingo-harness/run-suite.sh \
      /opt/bingo-tests/*.sh
