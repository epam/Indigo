FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt upgrade -y
RUN apt install -y --no-install-recommends \
        ca-certificates \
        curl \
        libfreetype6-dev \
        libfontconfig1-dev \
        python3 \
        python3-pip \
        python3-wheel \
        python3-setuptools \
        redis-server \
        supervisor \
        unzip \
        uwsgi \
        uwsgi-plugin-python3

# Install python dependencies using pip
COPY ./service/requirements.txt /opt/
RUN pip3 install --no-cache-dir -r /opt/requirements.txt

# Setup celery
COPY ./conf/celeryd.conf /etc/default/celeryd
RUN useradd -ms /bin/bash celery || echo "User already exists."
RUN chmod 640 /etc/default/celeryd
COPY ./conf/celery.auto.conf /etc/supervisor/conf.d/
# Setup redis
COPY ./conf/redis.auto.conf /etc/supervisor/conf.d/
# Setup uwsgi runner
COPY ./conf/uwsgi.ini /etc/uwsgi.ini
COPY ./conf/uwsgi.auto.conf /etc/supervisor/conf.d/

# Install Indigo
COPY ./lib/*indigo*linux*.whl /opt/
RUN if [ -f /opt/*indigo*manylinux1_x86_64*.whl ]; then python3 -m pip install --upgrade /opt/*indigo*manylinux1_x86_64*.whl; fi

# Install Imago
# COPY ./lib/imago-console-*.7z /opt/
# RUN cd /opt && \
#      mkdir /srv/imago && \
#      for imago_zip in $(cat /srv/service_version); do case "$imago_zip" in *imago*) 7z x $imago_zip; mv imago-console*/ /srv/imago/$(basename $imago_zip .7z)/ ;; esac; done
# RUN chmod -R a+rx /srv/imago

COPY ./lib/favicon.ico /srv/api/ui/
COPY ./service/v2/ /srv/api/v2/
COPY ./service/*.py /srv/api/

# Clean
RUN apt purge -y unzip git python3-pip python3-wheel && \
    apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*

EXPOSE 80
WORKDIR /srv/api
CMD gunicorn --bind 0.0.0.0:80 --workers=$(nproc) app:app
