from enum import Enum
from typing import Any, Dict, Generator, List, Optional, Tuple, Union

from elasticsearch import Elasticsearch
from elasticsearch.exceptions import NotFoundError, RequestError
from elasticsearch.helpers import streaming_bulk
from indigo import Indigo  # type: ignore

from bingo_elastic.model.record import (
    IndigoRecord,
    IndigoRecordMolecule,
    IndigoRecordReaction,
)
from bingo_elastic.queries import BaseMatch, query_factory
from bingo_elastic.utils import PostprocessType


class IndexName(Enum):
    BINGO_MOLECULE = "bingo-molecules"
    BINGO_REACTION = "bingo-reactions"


def get_index_name(record: IndigoRecord) -> IndexName:
    if isinstance(record, IndigoRecordMolecule):
        return IndexName.BINGO_MOLECULE
    if isinstance(record, IndigoRecordReaction):
        return IndexName.BINGO_REACTION
    raise AttributeError(f"Unknown IndigoRecord type {record}")


def get_record_by_index(
    response: Dict, index: str
) -> Union[IndigoRecordMolecule, IndigoRecordReaction]:
    if index == IndexName.BINGO_MOLECULE.value:
        return IndigoRecordMolecule(elastic_response=response)
    if index == IndexName.BINGO_REACTION.value:
        return IndigoRecordReaction(elastic_response=response)
    raise AttributeError(f"Unknown index {index}")


def elastic_repository_molecule(*args, **kwargs):
    return ElasticRepository(IndexName.BINGO_MOLECULE, *args, **kwargs)


def elastic_repository_reaction(*args, **kwargs):
    return ElasticRepository(IndexName.BINGO_REACTION, *args, **kwargs)


class ElasticRepository:
    def __init__(
        self,
        index_name: IndexName,
        *,
        host: Union[str, List[str]] = "localhost",
        port: int = 9200,
        scheme: str = "",
        http_auth: Optional[Tuple[str]] = None,
        ssl_context: Any = None,
        request_timeout: int = 60,
        retry_on_timeout: bool = True,
    ) -> None:
        """
        :param index_name: use function  get_index_name for setting this argument
        :param host: host or list of hosts
        :param port:
        :param scheme: http or https
        :param http_auth:
        :param ssl_context:
        :param timeout:
        :param retry_on_timeout:
        """
        arguments = {
            "port": port,
            "scheme": "https" if scheme == "https" else "http",
            "request_timeout": request_timeout,
            "retry_on_timeout": retry_on_timeout,
        }
        if isinstance(host, str):
            arguments["host"] = host
        else:
            arguments["hosts"] = host

        if http_auth:
            arguments["http_auth"] = http_auth

        if ssl_context:
            arguments["ssl_context"] = ssl_context

        self.index_name = index_name.value

        self.el_client = Elasticsearch(**arguments)  # type: ignore

    def __prepare(
        self, records: Generator[IndigoRecord, None, None]
    ) -> Generator[Dict, None, None]:
        for record in records:
            if get_index_name(record).value != self.index_name:
                raise ValueError(
                    f"Index {self.index_name} doesn't support store value "
                    f"of type {type(record)}"
                )
            yield record.as_dict()

    def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return self.index_records(gen(), chunk_size=1)

    def index_records(self, records: Generator, chunk_size: int = 500):
        self.create_index()
        # pylint: disable=unused-variable
        for is_ok, action in streaming_bulk(
            self.el_client,
            self.__prepare(records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass

    def create_index(self) -> None:
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
            assert isinstance(err_.info, dict)
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
        **kwargs,
    ) -> Generator[IndigoRecord, None, None]:

        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []

        query = self.__compile(
            similarity=similarity,
            exact=exact,
            substructure=substructure,
            limit=limit,
            postprocess_actions=postprocess_actions,
            **kwargs,
        )
        res = self.el_client.search(index=self.index_name, body=query)
        indigo_session = Indigo()
        for el_response in res.get("hits", {}).get("hits", []):
            record = get_record_by_index(el_response, self.index_name)
            for action_fn in postprocess_actions:
                record = action_fn(record, indigo_session)  # type: ignore
                if not record:
                    continue
            yield record

    # pylint: disable=no-self-use,too-many-arguments
    def __compile(
        self,
        similarity: BaseMatch = None,
        exact: IndigoRecord = None,
        substructure: IndigoRecord = None,
        limit: int = 10,
        postprocess_actions: PostprocessType = None,
        **kwargs,
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

        for key, value in kwargs.items():
            query_factory(key, value).compile(query)

        return query
