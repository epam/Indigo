ARG PG_VERSION=14

FROM postgres:${PG_VERSION}

ARG PG_VERSION

COPY ./dist/bingo-postgres${PG_VERSION}-linux-*.tgz /opt/

RUN cd /opt/ && tar -xzf bingo-postgres${PG_VERSION}*.tgz && \
mv /opt/bingo-postgres*/ /opt/bingo-postgres/ && \
cd /opt/bingo-postgres && \
sh ./bingo-pg-install.sh -libdir /opt/bingo-postgres/lib -y && \
chown postgres:postgres /opt/bingo-postgres/lib/libbingo-postgres.so && \
cp /opt/bingo-postgres/bingo_install.sql /docker-entrypoint-initdb.d/ && \
rm /opt/bingo-postgres*.tgz
