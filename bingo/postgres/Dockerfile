ARG BINGO_PG_VERSION

FROM postgres:${BINGO_PG_VERSION}

ARG BINGO_PG_VERSION

RUN echo BINGO_PG_VERSION=${BINGO_PG_VERSION}
COPY ./dist/bingo-postgres${BINGO_PG_VERSION}-linux-*.tgz /opt/

RUN cd /opt/ && tar -xzf bingo-postgres${BINGO_PG_VERSION}*.tgz && \
mv /opt/bingo-postgres*/ /opt/bingo-postgres/ && \
cd /opt/bingo-postgres && \
sh ./bingo-pg-install.sh -libdir /opt/bingo-postgres/lib -y && \
chown postgres:postgres /opt/bingo-postgres/lib/libbingo-postgres.so && \
cp /opt/bingo-postgres/bingo_install.sql /docker-entrypoint-initdb.d/ && \
rm /opt/bingo-postgres*.tgz
