# Self-contained Oracle XE + Bingo image. Bakes the prebuilt bingo-oracle .tgz
# into gvenzl/oracle-xe, installs the cartridge on first DB init, strips
# DISABLE_OOB on every startup.

FROM gvenzl/oracle-xe:21-slim

# Trailing slash on the COPY dest matters: the local builder appends a new
# tarball per run, so the glob can match several files. Copy to a directory
# and pick the newest in the RUN below.
COPY ./dist/bingo-oracle-linux-*.tgz /tmp/bingo-tgz/
COPY ./bingo/oracle/tests/docker/extproc.ora /opt/oracle/homes/OraDBHome21cXE/hs/admin/extproc.ora
COPY ./bingo/oracle/tests/docker/setup-sqlnet.sh /container-entrypoint-startdb.d/01-bingo-sqlnet.sh
COPY ./bingo/oracle/tests/docker/initdb-bingo.sh /container-entrypoint-initdb.d/01-bingo-install.sh

USER root
RUN set -eux && \
    mkdir -p /opt/bingo-oracle /opt/bingo-lib && \
    NEWEST=$(ls -t /tmp/bingo-tgz/bingo-oracle-linux-*.tgz | head -n 1) && \
    tar -xzf "$NEWEST" -C /opt/bingo-oracle --strip-components=1 && \
    cp /opt/bingo-oracle/lib/libbingo-oracle.so /opt/bingo-lib/ && \
    chown -R oracle:oinstall /opt/bingo-oracle /opt/bingo-lib && \
    chmod 644 /opt/bingo-lib/libbingo-oracle.so && \
    chown oracle:oinstall /opt/oracle/homes/OraDBHome21cXE/hs/admin/extproc.ora && \
    chmod 644 /opt/oracle/homes/OraDBHome21cXE/hs/admin/extproc.ora && \
    chmod 755 /container-entrypoint-startdb.d/01-bingo-sqlnet.sh \
              /container-entrypoint-initdb.d/01-bingo-install.sh && \
    rm -rf /tmp/bingo-tgz
USER oracle
