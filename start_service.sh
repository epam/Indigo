#!/usr/bin/env bash

cd utils/indigo-service/backend/service
python3 -m waitress --listen="127.0.0.1:8002" app:app
