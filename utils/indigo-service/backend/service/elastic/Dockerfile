FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt upgrade -y
RUN apt-get  -y install software-properties-common
RUN add-apt-repository -y ppa:deadsnakes/ppa
RUN apt install -y --no-install-recommends \
        ca-certificates \
        curl \
        libfreetype6-dev \
        libfontconfig1-dev \
        python3.9 \
        python3-pip \
        python3-wheel \
        python3-setuptools \
        redis-server \
        supervisor \
        unzip \
        uwsgi \
        uwsgi-plugin-python3

# Setup celery
COPY ./conf/celeryd.conf /etc/default/celeryd
RUN useradd -ms /bin/bash celery || echo "User already exists."
RUN chmod 640 /etc/default/celeryd
COPY ./conf/celery.auto.conf /etc/supervisor/conf.d/
# Setup redis
COPY ./conf/redis.auto.conf /etc/supervisor/conf.d/

# Setup gunicorn
COPY ./conf/gunicorn.auto.conf /etc/supervisor/conf.d/

COPY ./service/v3/ /srv/api/v3/
COPY ./service/*.py /srv/api/
COPY ./service/v3/common/*.py /srv/api/
COPY ./service/.env /srv/api/
RUN mkdir -p srv/api/upload
# Install python dependencies using pip
COPY ./service/requirements.txt /opt/
RUN pip3 install --no-cache-dir -r /opt/requirements.txt

# Clean
RUN apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*

EXPOSE 80
WORKDIR /srv/api
CMD supervisord -n
