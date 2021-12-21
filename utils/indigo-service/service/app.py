#!/usr/bin/env python

import logging
import sys
from argparse import ArgumentParser

from flasgger import Swagger
from flask import Flask
from werkzeug.serving import run_simple

# from v2.db.database import db_session
# from v2.imago_api import imago_api
from v2.common_api import common_api

# from v2.libraries_api import libraries_api
from v2.indigo_api import indigo_api

# def is_indigo_db():
#     try:
#         import socket
#         socket.gethostbyname('indigo_db')
#         return True
#     except:
#         return False


app = Flask(__name__)
app.config.from_pyfile("config.py")
# if is_indigo_db():
#     app.register_blueprint(libraries_api, url_prefix='/v2/libraries')
app.register_blueprint(indigo_api, url_prefix="/v2/indigo")
# app.register_blueprint(imago_api, url_prefix='/v2/imago')
app.register_blueprint(common_api, url_prefix="/v2")

swagger = Swagger(app)
# logging.basicConfig(, level=logging.INFO)
logging.basicConfig(
    stream=sys.stdout,
    format=u"[%(asctime)s: %(levelname)-8s/%(filename)s:%(lineno)d]  %(message)s",
    level=app.config.get("LOG_LEVEL"),
)


def run_server(port):
    run_simple(
        "0.0.0.0",
        port,
        app,
        use_reloader=True,
        use_debugger=True,
        use_evalex=True,
    )


# @app.teardown_appcontext
# def shutdown_session(exception=None):
#     db_session.remove()


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        "-s",
        "--server",
        action="store_true",
        dest="run_server",
        default=False,
        help="Run local server",
    )
    parser.add_argument(
        "-p",
        "--port",
        action="store",
        dest="port",
        type=int,
        default=5000,
        help="Specify port",
    )

    (options, args) = parser.parse_args()
    if options.run_server:
        run_server(options.port)
