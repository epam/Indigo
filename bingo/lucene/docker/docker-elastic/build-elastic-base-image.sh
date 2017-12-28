#!/usr/bin/env bash

docker build --no-cache=true -f Dockerfile-elastic-base -t indigo-lucene/elastic-base .
