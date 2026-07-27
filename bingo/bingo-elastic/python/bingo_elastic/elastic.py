from enum import Enum
from typing import (
    Any,
    AsyncGenerator,
    Dict,
    FrozenSet,
    Generator,
    List,
    Optional,
    Tuple,
    Type,
    TypeVar,
    Union,
)

from elasticsearch import Elasticsearch
from elasticsearch.exceptions import NotFoundError, RequestError
from elasticsearch.helpers import streaming_bulk

try:
    from elasticsearch import AsyncElasticsearch
    from elasticsearch.helpers import async_streaming_bulk
except ImportError:
    pass

from indigo import Indigo, IndigoObject  # type: ignore

from bingo_elastic.model.record import (
    RESERVED_FIELDS,
    IndigoRecord,
    IndigoRecordMolecule,
    IndigoRecordReaction,
)
from bingo_elastic.queries import BaseMatch, query_factory
from bingo_elastic.utils import PostprocessType

ElasticRepositoryT = TypeVar("ElasticRepositoryT")

# Mapping of custom (e.g. SDF tag) field name -> ES property mapping fragment
CustomPropertiesMapping = Dict[str, Dict[str, Any]]

MAX_ALLOWED_SIZE = 1000


class IndexName(Enum):
    def __init__(self, value):
        self._value_ = value

    BINGO_MOLECULE = "bingo-molecules"
    BINGO_REACTION = "bingo-reactions"
    BINGO_CUSTOM = "custom-index"

    def set_value(self, new_value):
        self._value_ = new_value


def get_index_name(record: IndigoRecord) -> IndexName:
    if isinstance(record, IndigoRecordMolecule):
        return IndexName.BINGO_MOLECULE
    if isinstance(record, IndigoRecordReaction):
        return IndexName.BINGO_REACTION
    if isinstance(record, str):
        return IndexName.BINGO_CUSTOM
    raise AttributeError(f"Unknown IndigoRecord type {record}")


def get_record_by_index(
    response: Dict, index: str
) -> Union[IndigoRecordMolecule, IndigoRecordReaction]:
    if index == IndexName.BINGO_MOLECULE.value:
        return IndigoRecordMolecule(elastic_response=response)
    if index == IndexName.BINGO_REACTION.value:
        return IndigoRecordReaction(elastic_response=response)
    if index == IndexName.BINGO_CUSTOM.value:
        return IndigoRecordMolecule(elastic_response=response)
    raise AttributeError(f"Unknown index {index}")


def elastic_repository_molecule(*args, **kwargs):
    return ElasticRepository(IndexName.BINGO_MOLECULE, *args, **kwargs)


def elastic_repository_reaction(*args, **kwargs):
    return ElasticRepository(IndexName.BINGO_REACTION, *args, **kwargs)


def get_client(
    *,
    client_type: Type[ElasticRepositoryT],
    host: Union[str, List[str]] = "localhost",
    port: int = 9200,
    scheme: str = "",
    http_auth: Optional[Tuple[str]] = None,
    ssl_context: Any = None,
    request_timeout: int = 60,
    retry_on_timeout: bool = True,
) -> ElasticRepositoryT:
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

    return client_type(**arguments)  # type: ignore


def build_index_body(
    tau_search: bool = False,
    custom_properties: Optional[CustomPropertiesMapping] = None,
) -> Dict:
    index_body = {
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
                "hash": {"type": "unsigned_long"},
                "has_error": {"type": "integer"},
            }
        }
    }

    if tau_search:
        index_body["mappings"]["properties"].update(
            {
                "tau_fingerprint": {
                    "type": "keyword",
                    "similarity": "boolean",
                },
                "tau_fingerprint_len": {"type": "integer"},
            }
        )

    if custom_properties:
        collisions = set(custom_properties).intersection(RESERVED_FIELDS)
        if collisions:
            raise ValueError(
                "custom_properties uses reserved field name(s): "
                f"{sorted(collisions)}"
            )
        index_body["mappings"]["properties"].update(custom_properties)

    return index_body


def non_indexed_fields(
    custom_properties: Optional[CustomPropertiesMapping],
) -> FrozenSet[str]:
    """Field names mapped with "index": false.

    Such fields are stored in _source (returned on retrieved records) but
    are not searchable, so filter() must reject queries against them.
    """
    if not custom_properties:
        return frozenset()
    return frozenset(
        name
        for name, fragment in custom_properties.items()
        if str(fragment.get("index", True)).lower() == "false"
    )


def validate_custom_properties(
    custom_properties: Optional[CustomPropertiesMapping],
) -> None:
    """custom_properties must map field names to ES mapping-fragment dicts."""
    if custom_properties is None:
        return
    if not isinstance(custom_properties, dict) or not all(
        isinstance(fragment, dict) for fragment in custom_properties.values()
    ):
        raise TypeError(
            "custom_properties must be a Dict[str, Dict[str, Any]] mapping "
            "field names to Elasticsearch property fragments"
        )


def check_index_exception(err_: RequestError) -> None:
    if not isinstance(err_.info, dict):
        raise err_
    cause = err_.info.get("error", {}).get("root_cause", [])
    if (
        len(cause) == 1
        and cause[0].get("type") == "resource_already_exists_exception"
    ):
        return
    raise err_


def create_index(
    index_name: str,
    el_client: Elasticsearch,
    *,
    index_body: Optional[Dict] = None,
) -> None:
    try:
        el_client.indices.create(index=index_name, body=index_body)
    except RequestError as err_:
        check_index_exception(err_)


async def a_create_index(
    index_name: str,
    el_client: "AsyncElasticsearch",
    *,
    index_body: Optional[Dict] = None,
) -> None:
    try:
        await el_client.indices.create(index=index_name, body=index_body)
    except RequestError as err_:
        check_index_exception(err_)


def prepare(
    records: Generator[IndigoRecord, None, None],
) -> Generator[Dict, None, None]:
    for record in records:
        # if get_index_name(record).value != index_name:
        #     raise ValueError(
        #         f"Index {index_name} doesn't support store value "
        #         f"of type {type(record)}"
        #     )
        yield record.as_dict()


def response_to_records(
    hits: dict,
    index_name: str,
    postprocess_actions: Optional[PostprocessType] = None,
    indigo_session: Optional[Indigo] = None,
    options: str = "",
    tests_yield_empty: bool = False,
) -> Generator[IndigoRecord, None, None]:
    for el_response in hits:
        record = get_record_by_index(el_response, index_name)
        for action_fn in postprocess_actions:  # type: ignore
            record = action_fn(record, indigo_session, options)  # type: ignore
            if not record:
                if tests_yield_empty:
                    yield IndigoRecordMolecule(empty=True)
                else:
                    break

        if record:
            yield record


class AsyncElasticRepository:
    def __init__(  # pylint: disable=too-many-arguments
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
        tau_search: bool = False,
        custom_properties: Optional[CustomPropertiesMapping] = None,
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
        :param tau_search: declare tau_fingerprint in the index mapping so
            tautomer-aware substructure search is available via
            filter(..., options="TAU ...")
        :param custom_properties: ES mapping fragments for caller-defined
            fields (SDF tags or kwargs passed to IndigoRecord). Keys are field
            names; values are ES property mappings, e.g.
            {"MolecularWeight": {"type": "float"}, "CAS": {"type": "keyword"}}.
            The same keys must also be passed as ``custom_properties=`` to
            iterate_sdf/iterate_file — without that, no SDF tags are
            extracted and the typed mapping has nothing to populate.
            Add ``"index": False`` to a fragment (e.g.
            {"comment": {"type": "keyword", "index": False}}) to store the
            value on records without making it searchable; filter() raises
            ValueError if such a field is queried.
        """
        self.index_name = index_name.value
        self.tau_search = tau_search
        self.custom_properties = custom_properties
        validate_custom_properties(custom_properties)
        self._non_indexed_fields = non_indexed_fields(custom_properties)
        self.index_body = build_index_body(tau_search, custom_properties)

        self.el_client = get_client(
            client_type=AsyncElasticsearch,
            host=host,
            port=port,
            scheme=scheme,
            http_auth=http_auth,
            ssl_context=ssl_context,
            request_timeout=request_timeout,
            retry_on_timeout=retry_on_timeout,
        )

    async def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return await self.index_records(gen(), chunk_size=1)

    async def index_records(self, records: Generator, chunk_size: int = 500):
        await a_create_index(
            self.index_name, self.el_client, index_body=self.index_body
        )
        # pylint: disable=unused-variable
        async for is_ok, action in async_streaming_bulk(
            self.el_client,
            prepare(records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass

    async def filter(  # pylint: disable=too-many-locals
        self,
        query_subject: Optional[
            Union[BaseMatch, IndigoObject, IndigoRecord]
        ] = None,
        indigo_session: Optional[Indigo] = None,
        limit: int = 10,
        keep_alive: str = "5m",
        page_size: int = 500,
        options: str = "",
        **kwargs,
    ) -> AsyncGenerator[IndigoRecord, None]:
        """
        Asynchronously filter records in Elasticsearch using a scroll-based search.

        This method performs a search query against the configured Elasticsearch index,
        returning a limited number of results through a scroll mechanism to efficiently
        handle large result sets.

        Args:
            query_subject: Input used to build the search query. Can be one of:
                - BaseMatch instance (e.g., substructure or similarity search)
                - IndigoObject instance (interpreted as a substructure search)
                - IndigoRecord subclass (interpreted as an exact search)
                - None or other types: query will be based on kwargs
            indigo_session: Indigo session
            limit: Maximum number of records to return. Since some results will
                be filtered out bythe postprocess actions, request to
                Elasticsearch server will use page_size limitation.
            keep_alive: Duration for which the scroll context should be kept alive
                (e.g., "5m" for 5 minutes).
            page_size: Number of records to fetch per scroll batch (max: MAX_ALLOWED_SIZE).
            options: Additional string-based options passed to postprocessing.
            **kwargs: Additional parameters passed to the query compiler.

        Yields:
            IndigoRecord objects matching the query, up to the specified limit.

        Raises:
            ValueError: If the page size exceeds the maximum allowed size.
            RuntimeError: If Elasticsearch does not return a scroll_id during a scroll operation.

        Notes:
            The scroll context is automatically cleared once iteration is complete or terminated.
            Although the scroll API is used here for deep pagination, it is no longer
            recommended by the Elasticsearch project. In specific cases it can consume
            significant resources.
            See: https://www.elastic.co/docs/reference/elasticsearch/rest-apis/
                paginate-search-results#scroll-search-results
        """
        tests_yield_empty: bool = kwargs.pop("tests_yield_empty", False)
        # this flag is used for testing. it will yield filtered out results with
        # empty=true attribute
        if page_size > MAX_ALLOWED_SIZE:
            raise ValueError(
                f"page_size should less or equal to {MAX_ALLOWED_SIZE}"
            )
        forbidden = self._non_indexed_fields.intersection(kwargs)
        if forbidden:
            raise ValueError(
                f"Field(s) {sorted(forbidden)} are mapped with index=false: "
                "stored on records but not searchable, so they cannot be "
                "used in filter()."
            )
        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []

        query = compile_query(
            query_subject=query_subject,
            limit=page_size,
            postprocess_actions=postprocess_actions,
            options=options,
            tau_search=self.tau_search,
            **kwargs,
        )
        try:
            scroll_id = None
            while limit > 0:
                if not scroll_id:
                    res = await self.el_client.search(
                        index=self.index_name,
                        body=query,
                        params={"scroll": keep_alive},
                    )
                else:
                    res = await self.el_client.scroll(
                        scroll_id=scroll_id, params={"scroll": keep_alive}
                    )

                hits = res.get("hits", {}).get("hits", [])
                if not hits:
                    break
                scroll_id = res.get("_scroll_id")
                if not scroll_id:
                    raise RuntimeError(
                        "Elasticsearch did not return a scroll_id. Unable to fetch data."
                    )

                for record in response_to_records(
                    hits,
                    self.index_name,
                    postprocess_actions,
                    indigo_session,
                    options,
                    tests_yield_empty=tests_yield_empty,
                ):
                    yield record
                    limit -= 1
                    if limit <= 0:
                        break
        finally:
            if scroll_id:
                await self.el_client.clear_scroll(scroll_id=scroll_id)

    async def close(self) -> None:
        await self.el_client.close()

    async def __aenter__(self, *args, **kwargs) -> "AsyncElasticRepository":
        return self

    async def __aexit__(self, *args, **kwargs) -> None:
        await self.close()


class ElasticRepository:
    def __init__(  # pylint: disable=too-many-arguments
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
        tau_search: bool = False,
        custom_properties: Optional[CustomPropertiesMapping] = None,
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
        :param tau_search: declare tau_fingerprint in the index mapping so
            tautomer-aware substructure search is available via
            filter(..., options="TAU ...")
        :param custom_properties: ES mapping fragments for caller-defined
            fields (SDF tags or kwargs passed to IndigoRecord). Keys are field
            names; values are ES property mappings, e.g.
            {"MolecularWeight": {"type": "float"}, "CAS": {"type": "keyword"}}.
            The same keys must also be passed as ``custom_properties=`` to
            iterate_sdf/iterate_file — without that, no SDF tags are
            extracted and the typed mapping has nothing to populate.
            Add ``"index": False`` to a fragment (e.g.
            {"comment": {"type": "keyword", "index": False}}) to store the
            value on records without making it searchable; filter() raises
            ValueError if such a field is queried.
        """
        self.index_name = index_name.value
        self.tau_search = tau_search
        self.custom_properties = custom_properties
        validate_custom_properties(custom_properties)
        self._non_indexed_fields = non_indexed_fields(custom_properties)
        self.index_body = build_index_body(tau_search, custom_properties)

        self.el_client = get_client(
            client_type=Elasticsearch,
            host=host,
            port=port,
            scheme=scheme,
            http_auth=http_auth,
            ssl_context=ssl_context,
            request_timeout=request_timeout,
            retry_on_timeout=retry_on_timeout,
        )

    def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return self.index_records(gen(), chunk_size=1)

    def index_records(self, records: Generator, chunk_size: int = 500):
        create_index(
            self.index_name, self.el_client, index_body=self.index_body
        )
        # pylint: disable=unused-variable
        for is_ok, action in streaming_bulk(
            self.el_client,
            prepare(records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass

    def delete_all_records(self):
        try:
            self.el_client.indices.delete(index=self.index_name)
        except NotFoundError:
            pass

    def filter(  # pylint: disable=too-many-locals
        self,
        query_subject: Optional[
            Union[BaseMatch, IndigoObject, IndigoRecord]
        ] = None,
        indigo_session: Optional[Indigo] = None,
        limit: int = 10,
        keep_alive: str = "5m",
        page_size: int = 500,
        options: str = "",
        **kwargs,
    ) -> Generator[IndigoRecord, None, None]:
        """
        Synchronously filter records in Elasticsearch using a scroll-based search.

        This method performs a search query against the configured Elasticsearch index,
        returning a limited number of results through a scroll mechanism to efficiently
        handle large result sets.

        Args:
            query_subject: Input used to build the search query. Can be one of:
                - BaseMatch instance (e.g., substructure or similarity search)
                - IndigoObject instance (interpreted as a substructure search)
                - IndigoRecord subclass (interpreted as an exact search)
                - None or other types: query will be based on kwargs
            indigo_session: Indigo session
            limit: Maximum number of records to return. Since some results
                will be filtered out bythe postprocess actions, request to
                Elasticsearch server will use page_size limitation.
            keep_alive: Duration for which the scroll context should be kept alive
                (e.g., "5m" for 5 minutes).
            page_size: Number of records to fetch per scroll batch (max: MAX_ALLOWED_SIZE).
            options: Additional string-based options passed to postprocessing.
            **kwargs: Additional parameters passed to the query compiler.

        Yields:
            IndigoRecord objects matching the query, up to the specified limit.

        Raises:
            ValueError: If the page size exceeds the maximum allowed size.
            RuntimeError: If Elasticsearch does not return a scroll_id during a scroll operation.

        Notes:
            The scroll context is automatically cleared once iteration is complete or terminated.
            Although the scroll API is used here for deep pagination, it is no longer
            recommended by the Elasticsearch project. In specific cases it can consume
            significant resources.
            See: https://www.elastic.co/docs/reference/elasticsearch/rest-apis/
                paginate-search-results#scroll-search-results
        """
        tests_yield_empty: bool = kwargs.pop("tests_yield_empty", False)
        # this flag is used for testing. it will yield filtered out results with
        # empty=true attribute
        if page_size > MAX_ALLOWED_SIZE:
            raise ValueError(
                f"page_size should less or equal to {MAX_ALLOWED_SIZE}"
            )
        forbidden = self._non_indexed_fields.intersection(kwargs)
        if forbidden:
            raise ValueError(
                f"Field(s) {sorted(forbidden)} are mapped with index=false: "
                "stored on records but not searchable, so they cannot be "
                "used in filter()."
            )
        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []
        query = compile_query(
            query_subject=query_subject,
            limit=min(limit, page_size),
            postprocess_actions=postprocess_actions,
            options=options,
            tau_search=self.tau_search,
            **kwargs,
        )

        try:
            scroll_id = None
            while limit > 0:
                if not scroll_id:
                    res = self.el_client.search(
                        index=self.index_name,
                        body=query,
                        params={"scroll": keep_alive},
                    )
                else:
                    res = self.el_client.scroll(
                        scroll_id=scroll_id, params={"scroll": keep_alive}
                    )

                hits = res.get("hits", {}).get("hits", [])
                if not hits:
                    break
                scroll_id = res.get("_scroll_id")
                if not scroll_id:
                    raise RuntimeError(
                        "Elasticsearch did not return a scroll_id. Unable to fetch data."
                    )

                for record in response_to_records(
                    hits,
                    self.index_name,
                    postprocess_actions,
                    indigo_session,
                    options,
                    tests_yield_empty=tests_yield_empty,
                ):
                    yield record
                    limit -= 1
                    if limit <= 0:
                        break
        finally:
            if scroll_id:
                self.el_client.clear_scroll(scroll_id=scroll_id)


def compile_query(
    query_subject: Optional[
        Union[BaseMatch, IndigoObject, IndigoRecord]
    ] = None,
    limit: int = 10,
    postprocess_actions: Optional[PostprocessType] = None,
    options: str = "",
    tau_search: bool = False,
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
                "tau_fingerprint",
                "tau_fingerprint_len",
            ],
        },
    }

    tau_mode = "TAU" in options.split()
    if tau_mode and not tau_search:
        raise ValueError(
            "TAU search requires tau_search=True on the repository "
            "(and on the indexed records). The tau_fingerprint field "
            "is not present on this index."
        )

    substructure_key = "tautomer" if tau_mode else "substructure"

    if isinstance(query_subject, BaseMatch):
        query_subject.compile(query, postprocess_actions)
    elif isinstance(query_subject, IndigoRecord):
        query_factory("exact", query_subject).compile(
            query, postprocess_actions
        )
    elif isinstance(query_subject, IndigoObject):
        query_factory(substructure_key, query_subject).compile(
            query, postprocess_actions
        )

    for key, value in kwargs.items():
        if tau_mode and key == "substructure":
            key = "tautomer"
        query_factory(key, value).compile(query, postprocess_actions)

    return query
