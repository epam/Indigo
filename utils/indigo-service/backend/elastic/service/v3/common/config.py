# Bingo config
import os

host_ip = os.environ.get("HOST_IP", "localhost")

# Flask config

MAX_CONTENT_LENGTH = 1024 * 1024 * 1024
UPLOAD_FOLDER = "/tmp/indigo-service/upload"
ALLOWED_EXTENSIONS = ("sdf", "sd", "gz")

# Celery config
CELERY_broker_url = f"redis://{host_ip}:6379/0"
result_backend = f"redis://{host_ip}:6379/0"
imports = "v3.libraries_api"
accept_content = ("json",)
task_serializer = "json"
result_serializer = "json"
CELERY_enable_utc = True
timezone = "Etc/UTC"
worker_pool = "prefork"

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

IMAGO_VERSIONS = ["2.0.0"]
