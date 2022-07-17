FROM node:12-slim as client-builder
WORKDIR /client
RUN apt update && \
    apt upgrade -y
COPY ./client /client
RUN npm install
RUN npm run gulp archive
RUN ls

FROM nginx:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt upgrade -y && \
    apt install -y --no-install-recommends \
      unzip

# Install client
COPY --from=client-builder /client/indigo-service-client*.zip /opt/
RUN cd /opt && \
    unzip indigo-service-client-*.zip && \
    mkdir -p /var/www/ && \
    mv indigo-service-client*/ /var/www/client/

# Install Ketcher
ADD https://github.com/epam/ketcher/releases/download/v2.5.1/ketcher-remote-2.5.1.zip /opt/ketcher-remote-2.5.1.zip
RUN cd /opt && \
    unzip ketcher*.zip && \
    mv remote/ /srv/ketcher/

COPY ./nginx/nginx.conf /etc/nginx/conf.d/default.conf

# Clean
RUN apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*
