# EPAM Indigo projects #

Copyright (c) 2009-2016 EPAM Systems
GNU General Public License version 3

## Introduction ##

This repository includes web client for [indigo-service](https://git.epam.com/epm-lsop/indigo-service) project

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
