FROM python:3.8

ENV PYTHONPATH=${PYTHONPATH}:${PWD} 
RUN pip install poetry
RUN poetry config virtualenvs.create false

WORKDIR /app

COPY src /app/src
COPY pyproject.toml /app 
RUN poetry install --no-dev

CMD uvicorn src:app --host 0.0.0.0 --port $PORT