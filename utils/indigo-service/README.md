# Overview

Indigo service provides a RESTful application for demo important Indigo toolkit functionality

* Ketcher
* Bingo cartridge
* Indigo API
* Imago OCR


# Style guides and approaches

## Binaries

Use EPAM artifactory for keeping binaries (> 1 Mb)!

## Code health

See [this](https://github.com/superbobry/pydonts) easily digestible guideline
for a start.

# Build

## Setting it up

First, make sure you have fairly recent version of
[Docker](https://docs.docker.com)
The required version is 1.9.0+ (or 1.6.0+ if you just use service) You may want to
additionally configure it to run commands without `sudo` (see Docker docs).
Next, you can build an images from root directory

## Install docker-compose

The general approach is to use `docker-compose` utility

https://docs.docker.com/compose/

You can install docker-compose as a python module

https://docs.docker.com/compose/install/

```
docker-compose $(CMD)
```

Or if you have just docker installed, use the following script (was downloaded from link above)

```
./run_dc.sh $(CMD)
```

## Command line

### Download all artifacts

```
git lfs install
git lfs pull
```

### Build images
```
docker-compose build
```

### Run containers (detached)
```
docker-compose up -d
```

### Stop containers
```
docker-compose stop
docker-compose stop $(NAME)
```
### Remove containers

```
docker-compose rm -f
```


# Dev mode

In dev mode the service directories are mounted from the working directory. Also, **uwsgi** runs with --py-autoreload option, which allows
apply the changes right after a service python code is modified

### Build images

```
docker-compose build debug indigo_service_front 
```

Run service in debug mode

```
INDIGO_SERVICE_PORT=80 docker-compose up debug indigo_service_front
```



# Tests

## Test data

`data` directory contains some test SD files. But you can also download some huge tests

```
cd ./tests/data/ && source download.sh
```

## Run unit tests

Run while running container

```
INDIGO_SERVICE_URL=http://localhost/v2 docker-compose up --build test
```

Run tests with ignore pattern

```
INDIGO_SERVICE_URL=http://localhost/v2  IGNORE_PATTERN=library_test docker-compose up  test
```

### Check if everything is working

    $ curl http://localhost:80/api/v2/info
    {
      "indigo_version": "1.2.1.188 linux64"
    }

## API Specification

API Specification should be available after building and launching the containers (uses Swagger UI). URL for docs:

    http://server_url/doc


# Useful commands

### Export with tag using `git` with `7z`

```
docker export indigoservice_service_1 > indigo_service_$(git describe --tags).tar
7z a -mx9 indigo_service_${version}.tar.7z indigo_service_${version}.tar
```

### Export using gzip

```
docker export indigoservice_service_1 | gzip  > indigo_service_$(git describe --tags).tar.gz
```

### Import

Local using 7z
```
7z x -so indigo_service_${version}.tar.7z | docker import -  indigo_service:${version}
```


Local using gzip

```
cat indigo_service_${version}.tar.gz | docker import -  indigo_service:${version}
```

From URL

```
docker import $(URL) - indigo_service:${version}
```

### Run imported container

```
docker run --restart=always -d -p 8002:8002 -e "INDIGO_UWSGI_RUN_PARAMETERS=--plugin python3" -e "PYTHONPATH=/srv/indigo-python" -e "PYTHONDONTWRITEBYTECODE=1" --name=indigo_service_${version}  indigo_service:${version} /bin/sh -c "supervisord -n"
```



Sometimes after changes in Dockerfile there are some errors because of the Docker
cache engine. You need to build the image using

    $ docker --no-cache=true build -t ...


Besides plain API you probably want to take a look at the cool web interface
we *temporary* bundle with the service as the only API-consumer. In that case
type `http://<docker-ip>:8000` in your browser.

### Configure new docker-compose.yml
Example of usage:
```
# ketcher-dev (no db)
INDIGO_SERVICE_PORT=8084 INDIGO_SERVICE_PYTHONPATH="/var/src/Indigo/build/indigo-python" docker-compose -p ketcher_dev up -d indigo_service indigo_service_front indigo_builder
# stable
INDIGO_SERVICE_PORT=8085 docker-compose -p stable up -d indigo_db indigo_service indigo_service_front

```


## Links

[win]: https://docs.docker.com/installation/windows/#container-port-redirection
[mac]: https://docs.docker.com/installation/mac/#access-container-ports
[mount]: https://docs.docker.com/userguide/dockervolumes/#mount-a-host-directory-as-a-data-volume
