FROM library/postgres:9.6

ENV POSTGRES_PASSWORD postgres

# Install
RUN apt-get update -qq && apt-get upgrade -qq -y
RUN apt-get install -y --no-install-recommends p7zip-full

# Copy cartridge
COPY ./lib/bingo-postgres*.7z /opt/
RUN cd /opt && \
    7z x bingo-postgres*.7z && \
	mv /opt/bingo-postgres*/ /opt/bingo-postgres/ && \
	chmod 777 -R /opt/bingo-postgres/

# Add init scripts
COPY ./db/configure_postgres.sh /docker-entrypoint-initdb.d/
COPY ./db/init_database.sql /opt/
COPY ./db/postgresql.conf /opt/
