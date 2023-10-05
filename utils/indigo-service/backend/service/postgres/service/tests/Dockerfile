FROM debian:stretch

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -qq

RUN apt-get install -y --no-install-recommends unzip python3 python3-pip python3-setuptools python3-wheel

RUN pip3 install requests==2.26

ENV INDIGO_UWSGI_RUN_PARAMETERS --plugin python3

# Install Indigo
COPY ./lib/indigo-python-* /opt/
RUN cd /opt && unzip indigo-python-* -d dist && mv dist/indigo-python-* /srv/indigo-python

COPY ./service/test /srv/api/test

WORKDIR /srv/api
