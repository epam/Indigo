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
# Setup uwsgi runner (Obsolete)
# COPY ./conf/uwsgi.ini /etc/uwsgi.ini
# COPY ./conf/uwsgi.auto.conf /etc/supervisor/conf.d/

# Setup gunicorn
COPY ./conf/gunicorn.auto.conf /etc/supervisor/conf.d/

# Install Imago
RUN mkdir -p /srv/imago/imago-console-2.0.0 && \
    cd /srv/imago/imago-console-2.0.0 && \
    curl https://lifescience.opensource.epam.com/downloads/imago-2.0.0/imago-2.0.0-linux64-console.zip -o imago-2.0.0-linux64-console.zip && \
    unzip imago-2.0.0-linux64-console.zip && \
    rm imago-2.0.0-linux64-console.zip && \
    chmod -R a+rx imago_console

# Install Indigo
COPY ./lib/* /opt/
RUN if [ -f /opt/*indigo*manylinux1_x86_64*.whl ]; then python3 -m pip install --upgrade /opt/*indigo*manylinux1_x86_64*.whl; rm /opt/*.whl; fi


# COPY ./lib/favicon.ico /srv/api/ui/
COPY ./service/v2/ /srv/api/v2/
COPY ./service/*.py /srv/api/
COPY ./service/v2/common/*.py /srv/api/

# Install python dependencies using pip
COPY ./service/requirements.txt /opt/
RUN pip3 install --no-cache-dir -r /opt/requirements.txt

# Clean
RUN apt purge -y unzip git python3-pip python3-wheel && \
    apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*

EXPOSE 80
WORKDIR /srv/api
# CMD gunicorn --bind 0.0.0.0:80 --workers=$(nproc) app:app
CMD supervisord -n
