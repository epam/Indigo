FROM maven:3.3

VOLUME /src
VOLUME /dist

COPY demo/webservice/docker/scripts/build-demo-webservice.sh /
RUN chmod +x build-demo-webservice.sh

CMD /build-demo-webservice.sh
