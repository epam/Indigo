FROM java:openjdk-8-jre
MAINTAINER  Artem Malykh "Artem_Malykh@epam.com"

ENV SOLR_VERSION 5.5.4
ENV SOLR solr-$SOLR_VERSION
ENV SOLR_USER solr
ENV SOLR_STARTUP_PARAMS ""

# Install solr
RUN export DEBIAN_FRONTEND=noninteractive && \
  apt-get update && \
  apt-get -y install lsof && \
  groupadd -r $SOLR_USER && \
  useradd -r -g $SOLR_USER $SOLR_USER && \
  mkdir -p /opt && \
  wget -v --output-document=/opt/$SOLR.tgz http://www.us.apache.org/dist/lucene/solr/$SOLR_VERSION/$SOLR.tgz && \
  tar -C /opt --extract --file /opt/$SOLR.tgz && \
  rm /opt/$SOLR.tgz && \
  ln -s /opt/$SOLR /opt/solr

RUN mkdir /opt/solr/plugin && \
    mkdir /opt/solr/conf && \
    mkdir /opt/solr/server/solr/moldocs/lib -p && \
    mkdir /opt/solr/server/logs -p && \
    touch /opt/solr/server/logs/solr.log

COPY scripts/indigo-lucene-startup.sh /opt/solr/startup.sh

RUN chmod +x /opt/solr/startup.sh

#RUN mkdir /opt/solr/1

EXPOSE 8983

WORKDIR /opt/solr
