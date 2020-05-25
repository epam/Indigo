FROM library/nginx:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && apt-get install -y --no-install-recommends unzip

# Install swagger
COPY ./lib/swagger-ui-*.zip /opt/
RUN cd /opt && \
    unzip swagger-ui-* && \
    mkdir -p /var/www && \
    mv doc/ /var/www/

# Install client
COPY ./lib/indigo-service-client-*.zip /opt/
RUN cd /opt && \
    unzip indigo-service-client-* && \
    mkdir -p /var/www/ && \
    mv indigo-service-client*/ /var/www/client/

# Install Ketcher
COPY ./lib/ketcher*.zip /opt/
RUN cd /opt && \
    unzip ketcher* && \
    mv ketcher*/ /srv/ketcher/

COPY ./lib/favicon.ico /var/www/client/
COPY ./nginx/nginx.conf /etc/nginx/conf.d/default.conf

# Clean
RUN apt-get autoremove -y && rm -rf /opt/* /var/lib/apt/lists/*


