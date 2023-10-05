FROM node:14-slim as ui-builder
RUN apt update && \
    apt upgrade -y
#RUN npm install -g yarn
RUN mkdir -p /var/ui
COPY ./ui/build /var/ui

FROM nginx:latest

ENV DEBIAN_FRONTEND=noninteractive

# Install UI
COPY --from=ui-builder /var/ui /opt
RUN cd /opt && \
    mkdir -p /var/www/
RUN mv /opt /var/www/ui


COPY ./nginx/nginx.conf /etc/nginx/conf.d/default.conf

# Clean
RUN apt autoremove -y && \
    rm -rf /opt/* /var/lib/apt/lists/*
