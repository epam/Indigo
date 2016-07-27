FROM maven:3.3

RUN mkdir /indigo-lucene && \
    mkdir /dist && \
    mkdir /build-status

VOLUME /build-status

COPY docker/scripts/build-all.sh /
COPY docker/scripts/build-chem-query.sh /

CMD /build-all.sh