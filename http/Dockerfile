FROM python:3.8-slim-buster

RUN apt-get update && apt-get install -y libcairo2-dev libfreetype6 libfontconfig

ENV PYTHONPATH=${PYTHONPATH}:${PWD} 
RUN pip install poetry
RUN poetry config virtualenvs.create false

WORKDIR /app

COPY pyproject.toml /app 
RUN poetry install --no-dev

COPY src /app/src

CMD uvicorn src:app --host 0.0.0.0 --port $PORT