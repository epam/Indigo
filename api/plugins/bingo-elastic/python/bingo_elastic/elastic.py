from typing import Dict, Generator, Optional, TypeVar, Union

from elasticsearch import Elasticsearch
from elasticsearch.exceptions import NotFoundError, RequestError
from elasticsearch.helpers import parallel_bulk, streaming_bulk

from bingo_elastic.model.record import IndigoRecord

SIM_FINGERPRINT = "sim_fingerprint"
SIM_FINGERPRINT_LEN = "sim_fingerprint_len"
SUB_FINGERPRINT = "sub_fingerprint"
SUB_FINGERPRINT_LEN = "sub_fingerprint_len"
CMF = "cmf"
NAME = "name"


class ElasticRepository:

    index_name = "bingo"

    def __init__(
        self,
        *,
        host: Union[str, list[str]] = "localhost",
        port: int = 9300,
        scheme: str = ""
    ) -> None:
        """
        :param host: host or list of hosts
        :param port:
        :param scheme: http or https
        """
        args = {
            "port": port,
            "scheme": "https" if scheme == "https" else "http",
        }
        if type(host) == str:
            args["host"] = host
        else:
            args["hosts"] = host

        self.el_client = Elasticsearch(**args)

    @staticmethod
    def __prepare(
        records: Generator[IndigoRecord, None, None]
    ) -> Generator[Dict, None, None]:
        for record in records:
            yield record.as_dict()

    def index_records(self, records: Generator, chunk_size: int = 500):
        self.create_index()
        for ok, action in streaming_bulk(
            self.el_client,
            self.__prepare(records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass
            # TODO: add post processing

    def index_records_parallel(
        self,
        records: Generator,
        chunk_size: int = 500,
        thread_count: int = 4,
    ) -> None:
        self.create_index()
        yield from parallel_bulk(
            self.el_client,
            records,
            chunk_size=chunk_size,
            thread_count=thread_count,
        )

    def create_index(self) -> None:
        # TODO: add number of shards
        # TODO: check index, compare with java version
        body = {
            "mappings": {
                "properties": {
                    SIM_FINGERPRINT: {
                        "type": "keyword",
                        "similarity": "boolean",
                    },
                    SIM_FINGERPRINT_LEN: {"type": "integer"},
                    SUB_FINGERPRINT: {
                        "type": "keyword",
                        "similarity": "boolean",
                    },
                    SUB_FINGERPRINT_LEN: {"type": "integer"},
                    CMF: {"type": "binary"},
                }
            }
        }
        try:
            self.el_client.indices.create(index=self.index_name, body=body)
        except RequestError as err_:
            cause = err_.info.get("error", {}).get("root_cause", [])
            if (
                len(cause) == 1
                and cause[0].get("type") == "resource_already_exists_exception"
            ):
                return
            raise err_

    def delete_all_records(self):
        try:
            self.el_client.indices.delete(index=self.index_name)
        except NotFoundError:
            pass

    def filter(self, *args):
        # todo: return Filter
        el_filter = Filter(self)


class Filter:
    def __init__(self, rep: ElasticRepository):
        self.rep = rep
        self.filter_ = {}

    def filter(self, *args):
        pass

    def collect(self):
        res = self.__compile()
        pass

    def __compile(self):
        pass
