import gzip
import json
import logging
import os
import re
import subprocess
import traceback
import types
import zlib
from time import time

import flask_restful
import redis
from flask import Blueprint, Response, request
from flask_httpauth import HTTPBasicAuth
from indigo import Indigo, IndigoException
from indigo.inchi import IndigoInchi
from indigo.renderer import IndigoRenderer
from marshmallow.exceptions import ValidationError
from pyparsing import ParseException

import config

from .celery_app import celery
from .common.util import api_route, item_to_sdf_chunk, merge_dicts
from .db.BingoPostgresAdapter import BingoPostgresAdapter
from .db.database import db_session
from .db.models import LibraryMeta
from .db.models import User as Usermodel
from .validation import LibrarySchema, SearcherSchema, UserSchema

libraries_api = Blueprint("libraries_api", __name__)

if not os.path.exists(config.__dict__["UPLOAD_FOLDER"]):
    os.makedirs(config.__dict__["UPLOAD_FOLDER"])
libraries_api.indigo = Indigo()
libraries_api.renderer = IndigoRenderer(libraries_api.indigo)
libraries_api.indigo_inchi = IndigoInchi(libraries_api.indigo)
libraries_api.config = config.__dict__
libraries_api.adapter = BingoPostgresAdapter(
    libraries_api.config, libraries_api.indigo, libraries_api.indigo_inchi
)
libraries_api.redis = redis.StrictRedis(host="localhost", port=6379, db=0)
libraries_api_app = flask_restful.Api(libraries_api)
libraries_api_logger = logging.getLogger("libraries")
# libraries_api_logger.addHandler(logging.FileHandler('/srv/api/app.log'))
auth = HTTPBasicAuth()
libraries_api_app.route = types.MethodType(api_route, libraries_api_app)


@auth.verify_password
def verify_password(email, password):
    user = Usermodel.query.filter(Usermodel.email == email).first()
    if not user or not user.check_password(password):
        return False
    return True


def _prepare_row(row):
    props = row[2]
    result_props = {}
    for p in props:
        result_props[p["a"]] = p["b"]
    record = {
        "id": row[0],
        "structure": zlib.decompress(row[1], 16 + zlib.MAX_WBITS),
        "properties": result_props,
        "library_id": row[3],
    }
    if len(row) == 5:
        record["found_properties_keys"] = row[4]
    return record


@celery.task(bind=True)
def search_total(self, params):
    try:
        params.update({"total": True})
        result = libraries_api.adapter.do_search(params)
        total = 0
        search_result = []
        for item in result:
            structure_ids = item[1]
            total += len(structure_ids)
            search_result.append(
                {"library_id": item[0], "structures": structure_ids}
            )
        params.pop("total", None)
        params.pop("limit", None)
        params.pop("offset", None)
        # print(params, search_result)
        libraries_api.redis.hmset(
            ":".join(["search", self.request.id]),
            {
                "parameters": json.dumps(params),
                "results": json.dumps(search_result),
                "count": total,
            },
        )
        return {"count": total}
    except Exception as e:
        self.update_state("FAILURE", meta={"error": str(e)})
        return {"error": "Internal server error"}, 500


@libraries_api_app.route("/search")
class Searcher(flask_restful.Resource):
    def post(self):
        """
        Search structures
        """
        libraries_api_logger.info(
            "[REQUEST] POST /search {0}".format(request.data)
        )
        try:
            input_dict = json.loads(request.data.decode("utf-8"))
        except ValueError:
            return {
                "error": "Invalid input JSON: {0}".format(request.data)
            }, 400
        try:
            search_params = SearcherSchema(strict=True).load(input_dict).data
            for library_id in search_params["library_ids"]:
                if not LibraryMeta.query.filter(
                    LibraryMeta.library_id == library_id
                ).first():
                    return {"error": "Library does not exist"}, 404
            task = search_total.apply_async((search_params,))
            cursor = libraries_api.adapter.do_search(search_params)
            results = []
            for row in cursor:
                item = _prepare_row(row)
                item["structure"] = item["structure"].decode()
                results.append(item)
            libraries_api_logger.info(
                "[RESPONSE] POST /search found {0} items".format(len(results))
            )
            return {"result": results, "search_id": task.id}
        except ValidationError as e:
            libraries_api_logger.error(
                "[RESPONSE-400] validation error: {0}".format(e.messages)
            )
            return {"error": e.messages}, 400
        except ParseException as e:
            libraries_api_logger.error(
                "[RESPONSE-422] query parser error: {0}".format(e.msg)
            )
            return {"error": {"query_text": "Incorrect query syntax."}}, 422
        except IndigoException as e:
            libraries_api_logger.error(
                "[RESPONSE-500] Indigo error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Indigo engine failed."}, 500
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@libraries_api_app.route("/search/<search_id>")
class SearcherExporter(flask_restful.Resource):
    def get(self, search_id):
        libraries_api_logger.info("[REQUEST] GET /search/{}".format(search_id))
        try:
            spec = search_id.split(".")
            search_id = spec[0]
            if len(spec) > 1 and spec[1] == "sdf":
                params = libraries_api.redis.hmget(
                    ":".join(["search", search_id]), "parameters"
                )
                if not params[0]:
                    return {"error": "Not found."}, 404
                libraries_api_logger.info(
                    "retrieved search params: {}".format(params)
                )
                cursor = libraries_api.adapter.do_search(json.loads(params[0]))

                def generate():
                    for row in cursor:
                        item = _prepare_row(row)
                        yield item_to_sdf_chunk(item)

                return Response(generate(), mimetype="chemical/x-mdl-sdfile")
            else:
                task = search_total.AsyncResult(search_id)
                response = {"state": task.state}
                if task.info:
                    response["result"] = task.info
                libraries_api_logger.info("[RESPONSE-200] {}".format(response))
                return response
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@libraries_api_app.route("/libraries")
class LibraryList(flask_restful.Resource):
    def get(self):
        """
        Get list of existing libraries
        """
        libraries_api_logger.info("[REQUEST] GET /libraries")
        try:
            res = []
            for lib in LibraryMeta.query.all():
                res.append(
                    merge_dicts({"id": lib.library_id}, lib.service_data)
                )
            return res
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500

    def post(self):
        """
        Create new library
        """
        libraries_api_logger.info(
            "[REQUEST] POST /libraries {0}".format(request.data)
        )
        try:
            input_dict = json.loads(request.data.decode("utf-8"))
        except ValueError:
            return {
                "error": "Invalid input JSON: {0}".format(request.data)
            }, 400
        try:

            data = LibrarySchema(strict=True).load(input_dict).data
            library_id = libraries_api.adapter.library_create(
                data["name"], data["user_data"]
            )
            libraries_api_logger.info("library_id: {0}".format(library_id))
            return (
                {"id": library_id},
                201,
                {
                    "Location": libraries_api_app.url_for(
                        Library, library_id=library_id, _external=True
                    )
                },
            )
        except ValidationError as e:
            libraries_api_logger.error(
                "[RESPONSE-400] validation error: {0}".format(e.messages)
            )
            return {"error": e.messages}, 400
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@libraries_api_app.route("/libraries/<library_id>")
class Library(flask_restful.Resource):
    def get(self, library_id=None):
        """
        Get information about existing library for current library_id
        """
        libraries_api_logger.info(
            "[REQUEST] GET /libraries/{0}".format(library_id)
        )
        if not library_id:
            return {"error": "Libary ID should be specified"}, 400
        if not LibraryMeta.query.filter(
            LibraryMeta.library_id == library_id
        ).first():
            return {"error": "Library does not exist"}, 404
        try:
            return libraries_api.adapter.library_get_info(library_id)
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500

    def put(self, library_id):
        libraries_api_logger.info(
            "[REQUEST] PUT /libraries/{} {}".format(library_id, request.data)
        )
        if not LibraryMeta.query.filter(
            LibraryMeta.library_id == library_id
        ).first():
            return {"error": "Library does not exist"}, 404
        try:
            data = json.loads(request.data.decode("utf-8"))
        except ValueError:
            return {
                "error": "Invalid input JSON: {0}".format(request.data)
            }, 400
        if "name" in data and not data["name"]:
            return {"error": {"name": "Library name cannot be empty."}}, 400
        try:
            return {
                "status": libraries_api.adapter.library_update(
                    library_id, data
                )
            }
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500

    def delete(self, library_id):
        """
        Delete existing library
        """
        libraries_api_logger.info(
            "[REQUEST] DELETE /libraries/{0}".format(library_id)
        )
        if not LibraryMeta.query.filter(
            LibraryMeta.library_id == library_id
        ).first():
            return {"error": "Library does not exist"}, 404
        try:
            return {"status": libraries_api.adapter.library_delete(library_id)}
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@celery.task(bind=True)
def import_file(self, library_id, path):
    try:
        # TODO: Improve incorrect file detection
        LibraryUpload.drop_indices(library_id)
        self.update_state(state="PROCESSING", meta={"stage": "inserting"})
        result_dict = LibraryUpload.external_insert(library_id, path)
        self.update_state(
            state="PROCESSING", meta=result_dict.update({"stage": "indexing"})
        )
        index_time = LibraryUpload.index_table(library_id)
        result_dict["index_time"] = round(index_time, 3)
        result_dict["index_speed"] = int(
            result_dict["structures_count"] / index_time
        )
        LibraryUpload.update_library_structures_count(
            library_id, result_dict["structures_count"]
        )
        self.update_state(state="SUCCESS", meta=result_dict)
        return result_dict
    except Exception as e:
        self.update_state(state="FAILURE", meta={"error": str(e)})
        return {"error": str(e)}, 500


@libraries_api_app.route("/libraries/<library_id>/uploads")
class LibraryUpload(flask_restful.Resource):
    @staticmethod
    def save_file(library_id, stream, mime_type):
        path = os.path.join(
            libraries_api.config["UPLOAD_FOLDER"],
            "{0}_{1}.{2}".format(library_id, int(time() * 1000), "sdf.gz"),
        )
        if mime_type == "chemical/x-mdl-sdfile":
            with gzip.open(path, "wb") as f:
                f.write(stream.read())
        else:
            with open(path, "wb") as f:
                f.write(stream.read())
        return path

    @staticmethod
    def external_insert(library_id, path):
        args = [
            os.path.join("/srv", "uploader", "indigo_uploader"),
            "-c",
            os.path.join("/srv", "uploader", "uploader_config.yml"),
            path,
            libraries_api.adapter.get_table_name_for_id(library_id),
        ]

        proc = subprocess.Popen(
            args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        out, err = proc.communicate()
        out = out.decode("utf-8")
        time_re = re.search(r"Insert total time = (\d+) ms", out)
        total_time = round(int(time_re.group(1)) / 1000.0, 3)
        speed_re = re.search(
            r"Average insert time = (\d+) structures per second", out
        )
        average_speed = int(speed_re.group(1))
        struct_re = re.search(r"Total structures processed = (\d+)", out)
        struct_count = int(struct_re.group(1))
        return {
            "insert_time": total_time,
            "structures_count": struct_count,
            "insert_speed": average_speed,
        }

    @staticmethod
    def drop_indices(library_id):
        libraries_api.adapter.drop_indices(
            libraries_api.adapter.get_table_name_for_id(library_id)
        )

    @staticmethod
    def index_table(library_id):
        start_time = time()
        libraries_api.adapter.create_indices(
            libraries_api.adapter.get_table_name_for_id(library_id)
        )
        return time() - start_time

    @staticmethod
    def update_library_structures_count(library_id, structures_count):
        data = libraries_api.adapter.library_get_info(library_id)
        service_data = data["service_data"]
        service_data["structures_count"] += structures_count
        service_data["updated_timestamp"] = int(time() * 1000)
        index_data = {
            "properties": libraries_api.adapter.library_get_properties(
                library_id
            )
        }
        return libraries_api.adapter.library_update(
            library_id, service_data, index_data
        )

    def post(self, library_id):
        """
        Upload and index file to the selected library_id
        """
        full_mime_type = request.headers.get("Content-Type")
        # optional parameters might be appended, get just the type
        mime_type = re.search(r"\A([^;]+)", full_mime_type).group(1)
        libraries_api_logger.info(
            "[REQUEST] POST /libraries/{0}/uploads Content-Type: {1}".format(
                library_id, mime_type
            )
        )
        allowed_types = (
            "chemical/x-mdl-sdfile",
            "application/x-gzip",
            "application/gzip",
        )
        if mime_type not in allowed_types:
            return {
                "error": "Incorrect Content-Type '{0}', should be one of [{1}]".format(
                    mime_type, ", ".join(allowed_types)
                )
            }, 415
        if not LibraryMeta.query.filter(
            LibraryMeta.library_id == library_id
        ).first():
            return {"error": "Library does not exist"}, 404
        try:
            path = self.save_file(library_id, request.stream, mime_type)
            upload = import_file.apply_async((library_id, path))
            return {"upload_id": upload.id}
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@libraries_api_app.route("/libraries/<library_id>/uploads/<upload_id>")
class LibraryUploadStatus(flask_restful.Resource):
    def get(self, library_id, upload_id):
        """
        Check upload status for selected library_id and upload_id
        """
        libraries_api_logger.info(
            "[REQUEST] GET /libraries/{0}/uploads/{1}".format(
                library_id, upload_id
            )
        )
        try:
            task = import_file.AsyncResult(upload_id)
            result_dict = {"state": task.state}
            if task.info:
                if (type(task) is list and "error" in task.info[0]) or (
                    type(task) is dict and "error" in task.info
                ):
                    libraries_api_logger.error(
                        "[RESPONSE-400] {0}".format(result_dict)
                    )
                    result_dict["metadata"] = task.info
                    return result_dict, 400
            libraries_api_logger.info("[RESPONSE-200] {0}".format(result_dict))
            return result_dict
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500


@libraries_api_app.route("/users")
class UsersList(flask_restful.Resource):
    @auth.login_required
    def get(self):
        libraries_api_logger.info("[REQUEST] GET /users")
        try:
            return libraries_api.adapter.user_all()
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500

    def post(self):
        libraries_api_logger.info(
            "[REQUEST] POST /users {0}".format(request.data)
        )
        try:
            input_dict = json.loads(request.data.decode("utf-8"))
        except ValueError:
            return {
                "error": "Invalid input JSON: {0}".format(request.data)
            }, 400
        try:
            input_dict = UserSchema(strict=True).load(input_dict).data
            if Usermodel.query.filter(
                Usermodel.email == input_dict["email"]
            ).first():
                return {
                    "error": {
                        "email": [
                            'Email "{}" is already used.'.format(
                                input_dict["email"]
                            )
                        ]
                    }
                }, 409
            u = self.user_create(input_dict)
            libraries_api_logger.info(
                "New user created, id={}".format(u.user_id)
            )
            return (
                {"id": u.user_id},
                201,
                {
                    "Location": libraries_api_app.url_for(
                        User, user_id=u.user_id, _external=True
                    )
                },
            )
        except ValidationError as e:
            libraries_api_logger.error(
                "[RESPONSE-400] validation error: {0}".format(e.messages)
            )
            return {"error": e.messages}, 400
        except Exception as e:
            libraries_api_logger.error(
                "[RESPONSE-500] internal error: {}\n{}".format(
                    str(e), traceback.format_exc()
                )
            )
            return {"error": "Internal server error."}, 500

    def user_create(self, params):
        u = Usermodel(params)
        db_session.add(u)
        db_session.commit()
        return u


@libraries_api_app.route("/users/<user_id>")
class User(flask_restful.Resource):
    def get(self, user_id):
        pass  # need to have this method for url_for(User, ...) to work
