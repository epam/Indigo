from celery import Celery  # type: ignore

from .common import config

celery = Celery(
    __name__,
    broker=config.CELERY_broker_url,
    backend=config.result_backend,
)
celery.conf.update(config.__dict__)
