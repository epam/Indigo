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
#
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
COPY setup.py ./

RUN pip3 install -r requirements_dev.txt && \
    pylint -sn --rcfile=pylintrc indigo_service tests && \
    mypy indigo_service && \
    pip3 install -e . && \
    pytest


FROM python:3.9-slim-buster

RUN mkdir -p /opt/indigo
WORKDIR /opt/indigo

COPY indigo_service ./indigo_service
COPY setup.py ./

RUN pip3 install .
# OVERRIDE ON ORCHESTRATION LEVEL
CMD indigo_service
