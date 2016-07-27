FROM indigo-lucene/base
MAINTAINER  Artem Malykh "Artem_Malykh@epam.com"

EXPOSE 6900
#Port for remote profiling
EXPOSE 6901
#Port for remote profiling
EXPOSE 7900

RUN chown -R $SOLR_USER:$SOLR_USER /opt/solr /opt/$SOLR

RUN echo "$SOLR_USER:solr" | chpasswd && adduser $SOLR_USER sudo && \
    echo "$SOLR_USER ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN printf "grant codebase \"file:/usr/lib/jvm/java-1.8.0-openjdk-amd64/lib/tools.jar\" {\n permission java.security.AllPermission;\n        };\n" > $JAVA_HOME/bin/jstatd.all.policy
RUN sed -i '/ENABLE_REMOTE_JMX_OPTS/d' /opt/solr/bin/solr.in.sh

ENV ENABLE_REMOTE_JMX_OPTS "true"
ENV RMI_PORT 6901
ENV SOLR_STARTUP_PARAMS "-a \"-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=6900\""

VOLUME /opt/solr/.m2

USER $SOLR_USER
WORKDIR /opt/solr

RUN mkdir indigo-lucene
RUN chown -R $SOLR_USER indigo-lucene

VOLUME /opt/solr/indigo-lucene
VOLUME /opt/solr/server/solr/moldocs

CMD ["/bin/bash", "./startup.sh"]
