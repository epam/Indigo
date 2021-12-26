from celery import Celery

import config

celery = Celery(
    __name__,
    broker="redis://localhost:6379/0",
    backend="redis://localhost:6379/0",
)
celery.conf.update(config.__dict__)
