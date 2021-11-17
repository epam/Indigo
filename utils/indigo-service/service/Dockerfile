FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y --no-install-recommends \
    unzip python3 python3-pip python3-wheel python3-setuptools libfreetype6-dev libfontconfig1-dev

# Write service version into file
# COPY ./ /opt/git/
# RUN cd /opt/git && \
#    git describe --tags >> /srv/service_version && \
#    find ./lib -name "imago-console-*" -exec basename -a {} + >> /srv/service_version

# Install python dependencies using pip
COPY ./service/requirements.txt /opt/
RUN pip3 install --no-cache-dir -r /opt/requirements.txt

# Install Celery
# COPY ./celery/celeryd.conf /etc/default/celeryd
# RUN useradd -ms /bin/bash celery || echo "User already exists."
# RUN chmod 640 /etc/default/celeryd
# COPY ./celery/celery.auto.conf /etc/supervisor/conf.d/

# Install redis runner
# COPY ./celery/redis.auto.conf /etc/supervisor/conf.d/

# Install uwsgi runner
# COPY ./uwsgi/uwsgi.ini /etc/uwsgi.ini
# COPY ./uwsgi/uwsgi.auto.conf /etc/supervisor/conf.d/
# ENV INDIGO_UWSGI_RUN_PARAMETERS --plugin python3

# Install Indigo
COPY ./lib/*indigo*linux*.whl /opt/
RUN python3 -m pip install /opt/*indigo*linux_amd64*.whl

# # Install Indigo Uploader
# COPY ./lib/indigo-uploader-*.zip /opt/
# RUN cd /opt && \
#      unzip indigo-uploader-* && \
#      mv indigo-uploader*/ /srv/uploader/
#
# COPY ./db/uploader_config.yml /srv/uploader/
# RUN mkdir -p /tmp/indigo-service/upload
# RUN chmod -R a+rw /tmp/indigo-service/
#
# # Install Imago
# COPY ./lib/imago-console-*.7z /opt/
# RUN cd /opt && \
#      mkdir /srv/imago && \
#      for imago_zip in $(cat /srv/service_version); do case "$imago_zip" in *imago*) 7z x $imago_zip; mv imago-console*/ /srv/imago/$(basename $imago_zip .7z)/ ;; esac; done
# RUN chmod -R a+rx /srv/imago

COPY ./lib/favicon.ico /srv/api/client/
COPY ./service/v2/ /srv/api/v2/
COPY ./service/*.py /srv/api/

# Clean
RUN apt-get purge -y unzip git python3-pip python3-wheel && \
    apt-get autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*

EXPOSE 80
WORKDIR /srv/api
CMD gunicorn --bind 0.0.0.0:80 --workers=5 app:app
