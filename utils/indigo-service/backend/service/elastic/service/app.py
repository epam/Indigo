#!/usr/bin/env python

import json
import logging
import sys

import urllib3 as urllib3
from dotenv import load_dotenv
from flask import Flask
from flask_cors import CORS
from v3.libraries_api import libraries_api
from werkzeug import run_simple

# Flask set-up
load_dotenv()
app = Flask(__name__)
CORS(app)
app.register_blueprint(libraries_api, url_prefix="/v3/libraries")
urllib3.disable_warnings()

logging.basicConfig(
    stream=sys.stdout,
    format="[%(asctime)s: %(levelname)-8s/%(filename)s:%(lineno)d]  %(message)s",
    level=app.config.get("LOG_LEVEL"),
)


@app.errorhandler(Exception)
def handle_exception(e):
    response = e.get_response()
    response.data = json.dumps(
        {
            "code": e.code,
            "name": e.name,
            "description": e.description,
        }
    )
    response.content_type = "application/json"
    return response, 500


if __name__ == "__main__":
    run_simple(
        "0.0.0.0",
        80,
        app,
        use_reloader=True,
        use_debugger=True,
        use_evalex=True,
    )
