# Bingo config

BINGO_POSTGRES = {
    "host": "indigo_db",
    "port": "5432",
    "database": "indigoservice",
    "user": "indigoservice",
    "password": "p@ssw0rd",
}

# Flask config

MAX_CONTENT_LENGTH = 1024 * 1024 * 1024
UPLOAD_FOLDER = "/tmp/indigo-service/upload"
ALLOWED_EXTENSIONS = ("sdf", "sd", "gz")

# Celery config

CELERY_BROKER_URL = "redis://localhost:6379/0"
CELERY_RESULT_BACKEND = "redis://localhost:6379/0"
# CELERY_IMPORTS = ('v2.imago_api', 'v2.libraries_api')
CELERY_ACCEPT_CONTENT = ("json",)
CELERY_TASK_SERIALIZER = "json"
CELERY_RESULT_SERIALIZER = "json"
CELERY_ENABLE_UTC = True
CELERY_TIMEZONE = "Etc/UTC"
CELERYD_POOL = "prefork"

# Logging option
LOG_LEVEL = "INFO"
# LOG_LEVEL = 'DEBUG'

# Swagger config

SWAGGER = {
    "swagger_version": "2.0",
    # headers are optional, the following are default
    "headers": [],
    "specs": [
        {
            "version": "0.2.0",
            "title": "Indigo Service API",
            "endpoint": "spec",
            "route": "/spec/",
        }
    ],
}

# imago config


ALLOWED_TYPES = (
    "image/png",
    "image/jpeg",
    "image/gif",
    "image/tiff",
    "image/bmp",
    "image/cmu-raster",
    "image/x-portable-bitmap",
)
