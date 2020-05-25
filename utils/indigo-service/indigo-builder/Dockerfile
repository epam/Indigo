FROM debian:stretch
ENV DEBIAN_FRONTEND noninteractive
ARG ARTIFACTORY_API_KEY

RUN apt-get update -qq && \
    apt-get install -qq -y --no-install-recommends curl ca-certificates unzip

RUN cd /opt && \
    curl -OL -H "X-JFrog-Art-Api:$ARTIFACTORY_API_KEY" -X GET "https://artifactory.epam.com/artifactory/EPM-LSOP/indigo/ci/indigo-python-latest-linux.zip" && \
    unzip indigo-python-* -d dist && \
    mkdir -p /var/src/Indigo/build/ && \
    mv dist/indigo-python-* /var/src/Indigo/build/indigo-python

RUN apt-get purge -qq -y curl ca-certificates unzip

