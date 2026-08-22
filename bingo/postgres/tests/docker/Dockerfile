# Build the exact Bingo PostgreSQL library from the checked-out source tree.
# Using the PostgreSQL 17 image as the builder base keeps pg_config, headers,
# and the runtime ABI aligned with the final test image.
FROM postgres:17-bookworm AS bingo-builder

USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential \
       ca-certificates \
       cmake \
       git \
       ninja-build \
       pkg-config \
       postgresql-server-dev-17 \
       python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# Bingo derives its version metadata from Git. Mark the copied checkout safe so
# the build cannot silently fall back to the bogus 1.9.1-00000000 version that
# older container builds reported when git describe was rejected.
RUN git config --global --add safe.directory /src \
    && test -d /src/.git \
    && git rev-parse HEAD > /tmp/source-revision \
    && (git describe --tags --always --dirty > /tmp/source-describe || git rev-parse --short HEAD > /tmp/source-describe)

RUN cmake -S /src -B /build-pg17 \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_BINGO_POSTGRES=ON \
      -DBUILD_INDIGO=OFF \
      -DBUILD_INDIGO_WRAPPERS=OFF \
      -DBUILD_BINGO_SQLSERVER=OFF \
      -DBUILD_BINGO_ORACLE=OFF \
      -DBUILD_BINGO_ELASTIC=OFF \
      -DBUILD_INDIGO_UTILS=OFF \
      -DPostgreSQL_ROOT=/usr \
    && cmake --build /build-pg17 \
      --target package-bingo-postgres \
      --parallel "$(nproc)"

# Use the package produced by Indigo itself as the handoff between stages. This
# verifies both the library build and the normal PostgreSQL packaging path.
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

RUN test -s /usr/lib/postgresql/17/lib/libbingo-postgres.so \
    && ldd /usr/lib/postgresql/17/lib/libbingo-postgres.so

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
