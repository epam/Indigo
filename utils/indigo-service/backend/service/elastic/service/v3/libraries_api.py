import gzip
import logging
import os
import re
import time
import traceback
import uuid
from pathlib import Path
from typing import Dict, Union

from bingo_elastic.elastic import ElasticRepository, IndexName  # type: ignore
from bingo_elastic.model import helpers  # type: ignore
from bingo_elastic.model.record import IndigoRecord  # type: ignore
from bingo_elastic.queries import (  # type: ignore
    BaseMatch,
    EuclidSimilarityMatch,
    TanimotoSimilarityMatch,
    TverskySimilarityMatch,
    query_factory,
)
from dotenv import load_dotenv  # type: ignore
from elasticsearch import Elasticsearch as NativeElastic  # type: ignore
from flask import Blueprint, request
from flask_cors import cross_origin
from indigo import Indigo, IndigoObject  # type: ignore

libraries_api = Blueprint("libraries_api", __name__)

# Indigo set-up
indigo = Indigo()
indigo.setOption("deco-ignore-errors", True)
indigo.setOption("ignore-stereochemistry-errors", True)

# Elastic set-up
load_dotenv()
save_folder_path = os.getenv("path_to_upload")
host = os.getenv("host")
port = os.getenv("port")
scheme = os.getenv("scheme")
history_index_name = "query_history"
comment_index_name = "lib_comment"
native_client = NativeElastic(f"{scheme}://{host}:{port}", verify_certs=False)
libraries_api_logger = logging.getLogger("libraries")


def bingo_repo():
    # Leave as is till to new release with custom index value
    return ElasticRepository(IndexName.BINGO_CUSTOM, host=host, port=port)


def set_up_index_value(id):
    try:
        IndexName.BINGO_CUSTOM.set_value(id)
    except Exception:
        traceback.print_exc()


def index(id, path):
    set_up_index_value(id)
    try:
        sdf = helpers.iterate_sdf(path)
        # NOTICE: elastic.py 147 line.
        bingo_repo().index_records(records=sdf)
    except Exception:
        traceback.print_exc()


def save_file(library_id, stream, mime_type):
    try:
        path = os.path.join(
            save_folder_path,
            "{0}_{1}.{2}".format(
                library_id, int(time.time() * 1000), "sdf.gz"
            ),
        )
        if mime_type == "chemical/x-mdl-sdfile":
            with gzip.open(path, "wb") as f:
                f.write(stream.read())
        else:
            with open(path, "wb") as f:
                f.write(stream.read())
    except Exception as e:
        raise e
    return path


@libraries_api.get("/libraries")
@cross_origin()
def lib_list():
    libraries_api_logger.info("[REQUEST] GET /libraries")
    result = []
    for lib in native_client.cat.indices(format="json", pretty=True):
        # 1. filter query_history index
        # 2. filter lib_comment index
        # 3. filter all indexes that start with "."
        if (
            lib["index"] == history_index_name
            or lib["index"] == comment_index_name
            or re.compile("^\\.").search(lib["index"])
        ):
            continue
        data = {
            "id": lib["index"],
            "name": lib["index"],
            "structures_count": lib["docs.count"],
        }
        result.append(data)
    return result, 200


@libraries_api.post("/libraries")
@cross_origin()
def add_new_lib():
    libraries_api_logger.info(
        "[REQUEST] POST /libraries {0}".format(request.json)
    )
    body = request.json
    native_client.indices.create(index=body["name"])
    comment_record = {
        "lib_id": body["name"],
        "comemnt": body["user_data"]["comment"],
    }
    native_client.index(
        index=comment_index_name, id=body["name"], document=comment_record
    )
    return {"id": body["name"]}, 201


@libraries_api.get("/libraries/<id>")
@cross_origin()
def lib_info(id):
    libraries_api_logger.info("[REQUEST] GET /libraries/{0}".format(id))
    index_row = native_client.cat.indices(
        index=id, format="json", pretty=True
    )[0]
    alias = native_client.get(
        index=comment_index_name, id=id, format="json", pretty=True
    )["_source"]["comemnt"]
    return {
        "service_data": {
            "name": index_row["index"],
            "structures_count": index_row["docs.count"],
            "created_timestamp": 0,
            "updated_timestamp": 0,
            "properties": {},
        },
        "user_data": {"comment": str(alias).replace("{", "").replace("}", "")},
    }, 200


@libraries_api.delete("/libraries/<id>")
@cross_origin()
def delete_lib(id):
    libraries_api_logger.info("[REQUEST] DELETE /libraries/{0}".format(id))
    set_up_index_value(id)
    bingo_repo().delete_all_records()
    return {"status": "OK"}, 200


@libraries_api.post("/libraries/<id>/uploads")
@cross_origin()
def upload_content_to_lib(id):
    libraries_api_logger.info(
        "[REQUEST] DELETE /libraries/{0}/uploads".format(id)
    )
    full_mime_type = request.headers.get("Content-Type")
    mime_type = re.search(r"\A([^;]+)", full_mime_type).group(1)

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

    if not native_client.indices.exists(index=id):
        return {"error": "Library does not exist"}, 404

    path = save_file(id, request.stream, mime_type)
    path_to_file = Path(path)
    try:
        index(id, path)
        return {"upload_id": id}, 200
    except Exception as e:
        return {
            "error": "Internal server error. Exception: {}.".format(e)
        }, 500
    finally:
        if path_to_file.exists():
            path_to_file.unlink()


@libraries_api.get("/<lib_id>/recount")
@cross_origin()
def recount(lib_id):
    libraries_api_logger.info(
        "[REQUEST] GET /libraries/{0}/recount".format(id)
    )
    native_client.count(index=lib_id)
    return "OK", 200


@libraries_api.post("/search")
@cross_origin()
def search():
    libraries_api_logger.info(
        "[REQUEST] POST /libraries/search {0}".format(request.json)
    )
    body = request.json

    ids = body["library_ids"]  # list
    limit = body["limit"]
    raw_query = body["query_structure"]
    type_val = body["type"]

    #   Question: does bingo support query_text?
    #   Answer: No, leave as comment
    #   query_text = body['query_text']

    search_id = str(uuid.uuid4())
    history_body = {
        "search_id": search_id,
        "libs": ids,
        "query": raw_query,
        # "query_text": query_text,
        # "offset": offset,
        "operation": type_val,
    }
    result_obj = []
    for id in ids:
        set_up_index_value(id)
        response = []

        if type_val == "exact":
            options = body["options"]
            history_body["options"] = options
            compound = indigo.loadMolecule(raw_query)
            rec = IndigoRecord(indigo_object=compound)
            response = bingo_repo().filter(
                query_subject=rec, limit=limit, options=options
            )
        if type_val == "sub":
            options = body["options"]
            history_body["options"] = options
            compound = indigo.loadQueryMolecule(raw_query)
            response = bingo_repo().filter(
                query_subject=compound,
                indigo_session=indigo,
                limit=limit,
                options=options,
            )
        if type_val == "sim":
            metric = body["metric"]
            threshold = body["min_sim"]
            history_body["metric"] = metric
            history_body["threshold"] = threshold
            if metric == "tanimoto":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                tanimoto = TanimotoSimilarityMatch(rec, threshold)
                response = bingo_repo().filter(
                    query_subject=tanimoto, indigo_session=indigo, limit=limit
                )
            if metric == "tversky":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                tversky = TverskySimilarityMatch(rec, threshold)
                response = bingo_repo().filter(
                    query_subject=tversky, indigo_session=indigo, limit=limit
                )
            if metric == "euclid-sub":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                euclid = EuclidSimilarityMatch(rec, threshold)
                response = bingo_repo().filter(
                    query_subject=euclid, indigo_session=indigo, limit=limit
                )

        for indigo_object in response:
            structure = {
                "id": indigo_object.record_id,
                "structure": indigo_object.as_indigo_object(indigo).molfile(),
                "library_id": id,
            }
            result_obj.append(structure)

    native_client.index(
        index=history_index_name, id=search_id, document=history_body
    )
    return {"result": result_obj, "search_id": search_id}, 200


@libraries_api.get("/search/<search_id>")
@cross_origin()
def search_count(search_id):
    libraries_api_logger.info(
        "[REQUEST] GET /libraries/search/{0}".format(search_id)
    )
    resp = native_client.get(index=history_index_name, id=search_id)
    content = resp["_source"]

    raw_query = content["query"]
    ids = content["libs"]
    type_val = content["operation"]

    start = time.time()
    total_count = 0
    for id in ids:
        query = ""
        if type_val == "exact":
            compound = indigo.loadMolecule(raw_query)
            rec = IndigoRecord(indigo_object=compound)
            query = compile_query(query_subject=rec)
        if type_val == "sub":
            compound = indigo.loadQueryMolecule(raw_query)
            query = compile_query(query_subject=compound)
        if type_val == "sim":
            metric = content["metric"]
            threshold = content["threshold"]
            if metric == "tanimoto":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                tanimoto = TanimotoSimilarityMatch(rec, threshold)
                query = compile_query(query_subject=tanimoto)
            if metric == "tversky":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                tversky = TverskySimilarityMatch(rec, threshold)
                query = compile_query(query_subject=tversky)
            if metric == "euclid-sub":
                compound = indigo.loadMolecule(raw_query)
                rec = IndigoRecord(indigo_object=compound)
                euclid = EuclidSimilarityMatch(rec, threshold)
                query = compile_query(query_subject=euclid)
        count_resp = native_client.search(
            index=id, body=query, track_total_hits=True
        )
        total_count += count_resp["hits"]["total"]["value"]
    end = time.time()
    total_time = round(((end - start) * (10**3)), 3)
    return {
        "state": "SUCCESS",
        "result": {"count": total_count, "time": total_time},
    }, 200


def compile_query(
    query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
    **kwargs,
) -> Dict:
    query: Dict = {}
    postprocess_actions: list = []
    if isinstance(query_subject, BaseMatch):
        query_subject.compile(query, postprocess_actions)
    elif isinstance(query_subject, IndigoRecord):
        query_factory("exact", query_subject).compile(
            query, postprocess_actions
        )
    elif isinstance(query_subject, IndigoObject):
        query_factory("substructure", query_subject).compile(
            query, postprocess_actions
        )

    for key, value in kwargs.items():
        query_factory(key, value).compile(query)

    return query


@libraries_api.get("/search/<search_id>.sdf")
def sdf_export(search_id):
    # TODO: extra-feature
    #  CHECK libraries_apy. SearcherExporter(flask_restful.Resource)
    return "sdf_export"
