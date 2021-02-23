from ast import Str
from typing import Any, Dict, Generator, List, Optional, Tuple, Union

from elasticsearch import Elasticsearch
from elasticsearch.exceptions import NotFoundError, RequestError
from elasticsearch.helpers import parallel_bulk, streaming_bulk
from indigo import Indigo

from bingo_elastic.model.record import IndigoRecord
from bingo_elastic.queries import BaseMatch, ExactMatch, query_factory
from bingo_elastic.utils import PostprocessType


class ElasticRepository:

    index_name = "bingo"

    def __init__(
        self,
        *,
        host: Union[str, List[Str]] = "localhost",
        port: int = 9200,
        scheme: Str = "",
        http_auth: Optional[Tuple[Str]] = None,
        ssl_context: Any = None
    ) -> None:
        """
        :param host: host or list of hosts
        :param port:
        :param scheme: http or https
        :param http_auth:
        :param ssl_context:
        """
        arguments = {
            "port": port,
            "scheme": "https" if scheme == "https" else "http",
        }
        if type(host) == str:
            arguments["host"] = host
        else:
            arguments["hosts"] = host

        if http_auth:
            arguments["http_auth"] = http_auth

        if ssl_context:
            arguments["ssl_context"] = ssl_context

        self.el_client = Elasticsearch(**arguments)

    @staticmethod
    def __prepare(
        records: Generator[IndigoRecord, None, None]
    ) -> Generator[Dict, None, None]:
        for record in records:
            yield record.as_dict()

    def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return self.index_records(gen(), chunk_size=1)

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

    # TODO: chunk size and thread count based on actual CPU values
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
                    "sim_fingerprint": {
                        "type": "keyword",
                        "similarity": "boolean",
                    },
                    "sim_fingerprint_len": {"type": "integer"},
                    "sub_fingerprint": {
                        "type": "keyword",
                        "similarity": "boolean",
                    },
                    "sub_fingerprint_len": {"type": "integer"},
                    "cmf": {"type": "binary"},
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

    def filter(
        self,
        similarity: Union[BaseMatch] = None,
        exact: IndigoRecord = None,
        substructure: IndigoRecord = None,
        limit=10,
        **kwargs
    ) -> Generator[IndigoRecord, None, None]:

        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []

        query = self.__compile(
            similarity=similarity,
            exact=exact,
            substructure=substructure,
            limit=limit,
            postprocess_actions=postprocess_actions,
            **kwargs
        )
        res = self.el_client.search(index=self.index_name, body=query)
        indigo_session = Indigo()
        for el_response in res.get("hits", {}).get("hits", []):
            record = IndigoRecord(elastic_response=el_response)
            for fn in postprocess_actions:
                record = fn(record, indigo_session)
                if not record:
                    continue
            yield record

    def __compile(
        self,
        similarity: BaseMatch = None,
        exact: IndigoRecord = None,
        substructure: IndigoRecord = None,
        limit: int = 10,
        postprocess_actions: PostprocessType = None,
        **kwargs
    ) -> Dict:

        query = {
            "size": limit,
            "_source": {
                "includes": ["*"],
                "excludes": [
                    "sim_fingerprint",
                    "sim_fingerprint_len",
                    "sub_fingerprint_len",
                    "sub_fingerprint",
                ],
            },
        }
        if similarity and substructure:
            # todo: enable search by similarity and substructure together
            # todo: add exact check also
            raise AttributeError(
                "similarity and substructure search " "is not supported"
            )

        if similarity:
            similarity.compile(query, postprocess_actions)
        elif exact:
            query_factory("exact", exact).compile(query, postprocess_actions)
        elif substructure:
            query_factory("substructure", substructure).compile(
                query, postprocess_actions
            )

        for k, v in kwargs.items():
            query_factory(k, v).compile(query)

        return query
