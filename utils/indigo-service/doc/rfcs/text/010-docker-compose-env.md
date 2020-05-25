- Start Date: 2015-01-14
- Implementation MR: !10
- Source issue: #36
- Status: 
- Scope: infrastructure

# Summary

* use docker-compose instead of direct docker containers.
* minimize image sizes. 
* split nginx and flask services
* remove java
* use artifactory to keep binaries

# Motivation

Right now there are lot of difficult commands to execute containers. We need to rememeber ports, volumes etc
Project is getting bigger and bigger and we need to minimize the compilation time and add new services (Imago) without losing simple processing


# Detailed design

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
cd ./lib/ && source download.sh
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


# Unresolved questions

* How to remove all links between containers?
* Hot to build binaries into artifactory?
* How to "lock" python envinronmet

# Things to do after merge

* Update README
* Remove files
   * db.Dockerfile
   * env.Dockerfile
   * Dockerfile
   * upload/
   * third_party/indigo
   * lib/*.zip
* Create compose files for DEV and TEST. [Details](https://docs.docker.com/compose/extends/#example-use-case)
* Update python libs for debian:jessie
* ...



