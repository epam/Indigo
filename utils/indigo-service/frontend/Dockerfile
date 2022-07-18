FROM node:12-slim as ui-builder
RUN apt update && \
    apt upgrade -y
COPY ./ui /ui
WORKDIR /ui
RUN npm install
RUN npm run gulp archive
RUN ls

FROM nginx:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt upgrade -y && \
    apt install -y --no-install-recommends \
      unzip

# Install UI
COPY --from=ui-builder /ui/indigo-service-ui*.zip /opt/
RUN cd /opt && \
    unzip indigo-service-ui-*.zip && \
    mkdir -p /var/www/ && \
    mv indigo-service-ui*/ /var/www/ui/

# Install Ketcher
#ADD https://github.com/epam/ketcher/releases/download/v2.5.1/ketcher-remote-2.5.1.zip /opt/ketcher-remote-2.5.1.zip
ADD https://github.com/epam/ketcher/releases/download/v2.1.0/ketcher-2.1.0.zip /opt/ketcher-2.1.0.zip
RUN cd /opt && \
    unzip ketcher*.zip && \
    # mv remote/ /srv/ketcher/ && \
    mv ketcher/ /srv/ketcher/ && \
    ls /srv/ketcher && \
    cp /srv/ketcher/ketcher.html /srv/ketcher/index.html

COPY ./nginx/nginx.conf /etc/nginx/conf.d/default.conf

# Clean
RUN apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*
