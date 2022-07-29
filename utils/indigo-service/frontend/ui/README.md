# EPAM Indigo projects #

Copyright (c) 2009-2022 EPAM Systems

## Introduction ##

This repository includes web UI for Indigo service

## Source code organization ##

## Build instructions ##

```
npm install && gulp
```

### Use docker

```
docker-compose up
```

## Run server with proxy path ##

```
npm install && gulp && gulp serve --api-proxy="http://indigoweb.epm-lsop.projects.epam.com"
```
