#!/usr/bin/env bash

#docker build --no-cache=true -f Dockerfile-elastic-base -t indigo-lucene/elastic-base .
docker build -f Dockerfile-elastic-base --tag=elastic-base .
docker run --rm -p 9200:9200 -p 9300:9300 -ti -v /usr/share/elasticsearch/data elastic-base