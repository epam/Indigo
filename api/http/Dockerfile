#
# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# For test purposes
FROM python:3.9-slim-buster as indigo_service_dev

RUN mkdir -p /opt/indigo
WORKDIR /opt/indigo

COPY pyproject.toml ./
COPY indigo_service ./indigo_service
COPY mypy.ini ./
COPY pylintrc ./
COPY requirements.txt ./
COPY requirements_dev.txt ./
COPY tests ./tests
# Optionally copy whl if exists. It's required for CI and local testing
COPY setup.py *manylinux1_x86_64*.whl ./

# Required for indigo-renderer
RUN apt-get update && \
    apt-get upgrade && \
    apt-get install -y --no-install-recommends libfreetype6-dev libfontconfig1-dev

# Install if exists
RUN if ls ./*manylinux1_x86_64*.whl 1> /dev/null 2>&1; then pip3 install ./*manylinux1_x86_64*.whl; fi

RUN pip3 install -r requirements.txt -r requirements_dev.txt && \
    pylint -sn --rcfile=pylintrc indigo_service tests && \
    mypy indigo_service && \
    pip3 install -e . && \
    pytest

# For final build
FROM python:3.9-slim-buster

RUN apt-get update && \
    apt-get upgrade && \
    apt-get install -y --no-install-recommends libfreetype6-dev libfontconfig1-dev

RUN mkdir -p /opt/indigo
WORKDIR /opt/indigo

COPY indigo_service ./indigo_service
COPY setup.py ./

# Cleanup
RUN apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

RUN pip3 install .
# OVERRIDE ON ORCHESTRATION LEVEL
CMD indigo_service
