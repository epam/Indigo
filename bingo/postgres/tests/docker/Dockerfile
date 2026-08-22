FROM postgres:17

# The harness intentionally tests the exact shared object that will be deployed.
# Copy a built libbingo-postgres.so into this directory before `docker compose build`.
COPY bingo/postgres/tests/docker/libbingo-postgres.so /usr/lib/postgresql/17/lib/libbingo-postgres.so
RUN mkdir -p /opt/bingo-harness \
    && test -s /usr/lib/postgresql/17/lib/libbingo-postgres.so \
    && ldd /usr/lib/postgresql/17/lib/libbingo-postgres.so \
    && sha256sum /usr/lib/postgresql/17/lib/libbingo-postgres.so \
       > /opt/bingo-harness/libbingo-postgres.sha256

# Generate the SQL bindings from the same source templates as the 1.45 package.
COPY bingo/postgres/setup/bingo-pg-install.sh /opt/bingo-package/bingo-pg-install.sh
COPY bingo/postgres/sql/ /opt/bingo-package/sql/
RUN mkdir -p /opt/bingo-package/lib \
    && cd /opt/bingo-package \
    && sh ./bingo-pg-install.sh -pglibdir -y \
    && cp bingo_install.sql /docker-entrypoint-initdb.d/00-bingo-install.sql

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
