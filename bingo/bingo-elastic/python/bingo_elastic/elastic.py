from __future__ import annotations

from enum import Enum
from typing import (
    Any,
    Dict,
    Generator,
    List,
    Optional,
    Tuple,
    Type,
    TypeVar,
    Union,
    Awaitable,
    Iterable,
    Iterator,
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
    IndigoRecord,
    IndigoRecordMolecule,
    IndigoRecordReaction,
)
from bingo_elastic.queries import BaseMatch, query_factory
from bingo_elastic.utils import PostprocessType

ElasticRepositoryT = TypeVar("ElasticRepositoryT")


class BingoElasticPageCriteria:
    """
    Captures the criteria to make a paged query in Bingo
    """

    _pit_id: Optional[str]
    _page_size: int
    _pit_stay_alive_minutes: int
    _sort: Optional[List[Dict[str, str]]]
    _search_after: Optional[List[Any]]
    _query: Optional[Dict[str, Any]]
    _next_page_search_after: List[Any]

    def to_json(self) -> Dict[str, Any]:
        """
        Provide ability to serialize this page criteria into a JSON for REST API clients.
        """
        return {
            "pit_id": self._pit_id,
            "page_size": self._page_size,
            "stay_alive_minutes": self._pit_stay_alive_minutes,
            "sort": self._sort,
            "search_after": self._search_after,
            "query": self._query,
        }

    @staticmethod
    def from_json(json_dct: Dict[str, Any]) -> BingoElasticPageCriteria:
        """
        Provide deserialization ability from page criteria to a JSON REST API client.
        """
        _pit_id: Optional[str] = json_dct.get("pit_id")
        _page_size: int = json_dct.get("page_size")
        _pit_stay_alive_minutes: int = json_dct.get("stay_alive_minutes")
        _sort: List[Dict[str, str]] = json_dct.get("sort")
        _search_after: List[Any] = json_dct.get("search_after")
        _query: Dict[str, Any] = json_dct.get("query")
        return BingoElasticPageCriteria(
            page_size=_page_size,
            pit_id=_pit_id,
            sort=_sort,
            pit_stay_alive_minutes=_pit_stay_alive_minutes,
            search_after=_search_after,
            query=_query,
        )

    @property
    def query(self) -> Optional[Dict[str, Any]]:
        """
        Get the precompiled query, which will be stored in the following pages of first page for performance.
        """
        return self._query

    @property
    def pit_id(self) -> Optional[str]:
        """
        Get the Point In Time (PIT) query identifier. The identifier must either be blank, or must be non-expired.
        """
        return self._pit_id

    @property
    def page_size(self) -> int:
        """
        The page size of the query total.
        Cannot exceed maximum 999 (1 extra is canary for testing next page availability).
        """
        return self._page_size

    @property
    def pit_stay_alive_minutes(self) -> int:
        """
        Get the Point In Time query stay alive minutes, which will be refreshed if there is another paged query again.
        Note the elasticsearch does not support floating point values.
        """
        return self._pit_stay_alive_minutes

    @property
    def sort_criteria(self) -> Optional[List[Dict[str, str]]]:
        """
        By default, the query will be sorted by score followed by PIT shard ID as tie-breaker implicitly.
        If an alternative sort order is desired, enter it here.
        """
        return self._sort

    @property
    def search_after(self) -> Optional[List[Any]]:
        """
        The cursor of the page we are retrieving of the previous record of the first record of this page.
        If this is the first page. This will be None.
        """
        return self._search_after

    def __init__(
        self,
        page_size: int = 10,
        pit_id: Optional[str] = None,
        sort: Optional[List[Dict[str, str]]] = None,
        pit_stay_alive_minutes: int = 30,
        search_after: Optional[List[Any]] = None,
        query: Optional[Dict[str, Any]] = None,
    ):
        """
        Create custom page criteria to query any particular page with particular number of records to skip.
        Note: in order to continue the query, the sort order must not be changed.
        https://www.elastic.co/guide/en/elasticsearch/reference/current/paginate-search-results.html
        :param page_size: The page size must not exceed the elastic page limit. The elastic page limit is adjustable
        by admin configuration and is usually default to 999 (with canary).
        Note: there will be one extra hit to be used for canary so this should be 1 less than the limit.
        :param pit_id: The PIT identifier for the current query. A fresh query with have this as None.
        :param sort: The optional sort order of the paged query. If none provided, this will be sort by relevance score.
        :param pit_stay_alive_minutes: the number of minutes the PIT (point in time) query will stay alive for continued
        browsing.
        :param search_after To continue querying, obtain the last record's sort result and append it in this parameter.
        """
        if not sort:
            sort = [{"_score": "desc"}]
        # shard_doc in sort is implicit
        self._page_size = page_size
        self._pit_id = pit_id
        self._sort = sort
        self._pit_stay_alive_minutes = pit_stay_alive_minutes
        self._search_after = search_after
        self._query = query


class BingoElasticPageResult(Awaitable, Iterable):
    """
    Result of a single page query in Bingo elastic.
    """

    _records_of_page: List[Optional[IndigoRecord]]
    _current_page_criteria: BingoElasticPageCriteria
    _num_hits_in_elastic: int
    _num_actual_hits: int
    _last_hit_sort_object: Optional[List[Any]]
    _gen: Generator[IndigoRecord, None, None]
    _completed_processing: bool

    def get_records(
        self, filter_false_positives: bool = True
    ) -> Tuple[Optional[IndigoRecord], ...]:
        """
        Get records in this page.
        :param filter_false_positives: If true, the hits in elastic that are filtered out by post-processor will
        not be returned. If false, null object will be returned in position where false positive had occurred.
        """
        if not filter_false_positives:
            return tuple(self._records_of_page)
        return tuple([x for x in self._records_of_page if x is not None])

    @property
    def current_page(self) -> BingoElasticPageCriteria:
        """
        Get the current page criteria for the query.
        """
        return self._current_page_criteria

    @property
    def num_hits_in_elastic(self) -> int:
        """
        Get number of hits in this page that's in elastic but may be false-positives.
        """
        return self._num_hits_in_elastic

    @property
    def num_actual_hits(self) -> int:
        """
        Get the number of actual hits after post-processing filter.
        Note: this can be size 1 greater than original page size if the canary for next page is hit.
        """
        return self._num_actual_hits

    @property
    def has_next_page(self) -> bool:
        """
        Use the canary to decide whether the next page is available or not.
        """
        # If there isn't any hit in ELASTIC page at all for some reason (i.e. first page no result) then no next page.
        if not self._completed_processing:
            raise AssertionError(
                "Cannot test next page availability using async I/O "
                "without fully retrieving current page result first.."
            )
        if not self._last_hit_sort_object:
            return False
        return (
            self._num_hits_in_elastic
            >= self._current_page_criteria.page_size + 1
        )

    @property
    def next_page_criteria(self) -> Optional[BingoElasticPageCriteria]:
        if not self.has_next_page:
            return None
        cur = self.current_page
        return BingoElasticPageCriteria(
            page_size=cur.page_size,
            pit_id=cur.pit_id,
            sort=cur.sort_criteria,
            pit_stay_alive_minutes=cur.pit_stay_alive_minutes,
            query=cur.query,
            search_after=self._last_hit_sort_object,
        )

    def synchronized(self) -> None:
        """
        Synchronize the object so we finish the processing completely
        """
        if self._completed_processing:
            return
        for record in self.__await__():
            pass

    def __init__(
        self,
        gen: Generator[IndigoRecord, None, None],
        current_page_criteria: BingoElasticPageCriteria,
    ):
        self._current_page_criteria = current_page_criteria
        self._gen = gen
        self._records_of_page = list()
        self._num_hits_in_elastic = 0
        self._num_actual_hits = 0
        self._completed_processing = False
        self._last_hit_sort_object = None

    def __iter__(self) -> Iterator[Optional[IndigoRecord]]:
        """
        Backward compatibility method to obtain iterator of indigo records.
        We track the iterator, so it never goes back like before, to mimic its behavior.
        """
        self.synchronized()
        return self.get_records(filter_false_positives=False).__iter__()

    def __await__(self) -> Generator[IndigoRecord, None, None]:
        for record in self._gen:
            self._num_hits_in_elastic += 1
            # Avoid returning the canary in the page.
            if (
                self._num_hits_in_elastic
                > self._current_page_criteria.page_size
            ):
                break
            # make sure we get canary of the last hit of actual last page instead of canary (must be after break)
            # noinspection PyProtectedMember
            self._last_hit_sort_object = (
                self._current_page_criteria._next_page_search_after
            )
            self._records_of_page.append(record)
            # If post-processing filtered it out then it's not an actual hit.
            if record is not None:
                self._num_actual_hits += 1
            yield record
        self._completed_processing = True


class IndexType(Enum):
    BINGO_MOLECULE = "bingo-molecules"
    BINGO_REACTION = "bingo-reactions"


def get_index_type(record: IndigoRecord) -> IndexType:
    if isinstance(record, IndigoRecordMolecule):
        return IndexType.BINGO_MOLECULE
    if isinstance(record, IndigoRecordReaction):
        return IndexType.BINGO_REACTION
    raise AttributeError(f"Unknown IndigoRecord type {record}")


def get_record_by_index(
    response: Dict, index_type: IndexType
) -> Union[IndigoRecordMolecule, IndigoRecordReaction]:
    if index_type == IndexType.BINGO_MOLECULE:
        return IndigoRecordMolecule(elastic_response=response)
    if index_type == IndexType.BINGO_REACTION:
        return IndigoRecordReaction(elastic_response=response)
    raise AttributeError(f"Unknown index {str(index_type)}")


def elastic_repository_molecule(index_name: str, *args, **kwargs):
    return ElasticRepository(
        IndexType.BINGO_MOLECULE, index_name, *args, **kwargs
    )


def elastic_repository_reaction(index_name: str, *args, **kwargs):
    return ElasticRepository(
        IndexType.BINGO_REACTION, index_name, *args, **kwargs
    )


def get_client(
    *,
    client_type: Type[ElasticRepositoryT],
    host: Union[str, List[str]] = "localhost",
    port: int = 9200,
    scheme: str = "",
    http_auth: Optional[List[str]] = None,
    ssl_context: Any = None,
    request_timeout: int = 60,
    timeout: int = 60,
    retry_on_timeout: bool = True,
) -> ElasticRepositoryT:
    arguments = {
        "port": port,
        "scheme": "https" if scheme == "https" else "http",
        "request_timeout": request_timeout,
        "retry_on_timeout": retry_on_timeout,
        "timeout": timeout,
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


index_body = {
    "mappings": {
        "properties": {
            "sim_fingerprint": {"type": "keyword", "similarity": "boolean"},
            "sim_fingerprint_len": {"type": "integer"},
            "sub_fingerprint": {"type": "keyword", "similarity": "boolean"},
            "sub_fingerprint_len": {"type": "integer"},
            "cmf": {"type": "binary"},
            "hash": {"type": "unsigned_long"},
            "has_error": {"type": "integer"},
        }
    }
}


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


def create_index(index_name: str, el_client: Elasticsearch) -> None:
    try:
        el_client.indices.create(index=index_name, body=index_body)
    except RequestError as err_:
        check_index_exception(err_)


async def a_create_index(
    index_name: str, el_client: "AsyncElasticsearch"
) -> None:
    try:
        await el_client.indices.create(index=index_name, body=index_body)
    except RequestError as err_:
        check_index_exception(err_)


def prepare(
    index_type: IndexType, records: Generator[IndigoRecord, None, None]
) -> Generator[Dict, None, None]:
    for record in records:
        if index_type != get_index_type(record):
            raise ValueError(
                f"Index {str(index_type)} doesn't support store value "
                f"of type {type(record)}"
            )
        yield record.as_dict()


def get_page_result(
    res: dict,
    index_type: IndexType,
    page_criteria: BingoElasticPageCriteria,
    postprocess_actions: PostprocessType = None,
    indigo_session: Indigo = None,
    options: str = "",
) -> BingoElasticPageResult:
    def page_result_gen() -> Generator[IndigoRecord, None, None]:
        for el_response in res.get("hits", {}).get("hits", []):
            record = get_record_by_index(el_response, index_type)
            for action_fn in postprocess_actions:  # type: ignore
                record = action_fn(
                    record, indigo_session, options
                )  # type: ignore
                if not record:
                    continue
            yield record

    return BingoElasticPageResult(
        gen=page_result_gen(), current_page_criteria=page_criteria
    )


class AsyncElasticRepository:
    def __init__(
        self,
        index_type: IndexType,
        index_name: str,
        *,
        host: Union[str, List[str]] = "localhost",
        port: int = 9200,
        scheme: str = "",
        http_auth: Optional[List[str]] = None,
        ssl_context: Any = None,
        request_timeout: int = 60,
        timeout: int = 60,
        retry_on_timeout: bool = True,
    ) -> None:
        """
        :param index_type: use function  get_index_name for setting this argument
        :param index_name: the name of the index
        :param host: host or list of hosts
        :param port:
        :param scheme: http or https
        :param http_auth:
        :param ssl_context:
        :param timeout:
        :param retry_on_timeout:
        """
        self.index_type = index_type
        self.index_name = index_type.value
        if index_name:
            self.index_name += "-" + index_name

        self.el_client = get_client(
            client_type=AsyncElasticsearch,
            host=host,
            port=port,
            scheme=scheme,
            http_auth=http_auth,
            ssl_context=ssl_context,
            request_timeout=request_timeout,
            timeout=timeout,
            retry_on_timeout=retry_on_timeout,
        )

    async def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return await self.index_records(gen(), chunk_size=1)

    async def index_records(self, records: Generator, chunk_size: int = 500):
        await a_create_index(self.index_name, self.el_client)
        # pylint: disable=unused-variable
        async for is_ok, action in async_streaming_bulk(
            self.el_client,
            prepare(self.index_type, records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass

    async def filter(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        indigo_session: Indigo = None,
        page_criteria: Optional[BingoElasticPageCriteria] = None,
        options: str = "",
        **kwargs,
    ) -> BingoElasticPageResult:
        """
        Return async page result without waiting for page's full post-processing to complete.
        The client is expected to consume the Awaitable object returned by using "await object" syntax
        or consume its generator directly (depending on the type of parallelism desired).
        """

        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []

        page_criteria = self.compile_query(
            query_subject=query_subject,
            page_criteria=page_criteria,
            postprocess_actions=postprocess_actions,
            **kwargs,
        )
        # We must NOT specify an index name as this is inherited by PIT.
        res = await self.el_client.search(body=page_criteria.query)
        ret = get_page_result(
            res,
            self.index_type,
            page_criteria,
            postprocess_actions,
            indigo_session,
            options,
        )
        return ret

    async def delete(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        limit: int = 1000,
        **kwargs,
    ) -> Dict[str, Any]:
        """
        Delete documents in index by a query filter.
        """
        if not self.el_client.indices.exists(index=self.index_name):
            return dict()
        page_criteria = self.compile_query(
            query_subject=query_subject,
            page_criteria=BingoElasticPageCriteria(page_size=limit - 1),
            is_delete_query=True,
            **kwargs,
        )
        return await self.el_client.delete_by_query(
            index=self.index_name, body=page_criteria.query, slices="auto"
        )

    async def close(self) -> None:
        await self.el_client.close()

    async def __aenter__(self, *args, **kwargs) -> "AsyncElasticRepository":
        return self

    async def __aexit__(self, *args, **kwargs) -> None:
        await self.close()

    def compile_query(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        page_criteria: Optional[BingoElasticPageCriteria] = None,
        postprocess_actions: PostprocessType = None,
        is_delete_query: bool = True,
        **kwargs,
    ) -> BingoElasticPageCriteria:
        return _compile_query(
            self.index_name,
            self.el_client,
            query_subject,
            page_criteria,
            postprocess_actions,
            is_delete_query,
            **kwargs,
        )


class ElasticRepository:
    def __init__(
        self,
        index_type: IndexType,
        index_name: str,
        *,
        host: Union[str, List[str]] = "localhost",
        port: int = 9200,
        scheme: str = "",
        http_auth: Optional[List[str]] = None,
        ssl_context: Any = None,
        request_timeout: int = 60,
        timeout: int = 60,
        retry_on_timeout: bool = True,
    ) -> None:
        """
        :param index_type: use function  get_index_name for setting this argument
        :param index_name: the name of this index after index type.
        :param host: host or list of hosts
        :param port:
        :param scheme: http or https
        :param http_auth:
        :param ssl_context:
        :param timeout:
        :param retry_on_timeout:
        """
        self.index_type = index_type
        self.index_name = index_type.value
        if index_name:
            self.index_name += "-" + index_name

        self.el_client = get_client(
            client_type=Elasticsearch,
            host=host,
            port=port,
            scheme=scheme,
            http_auth=http_auth,
            ssl_context=ssl_context,
            request_timeout=request_timeout,
            timeout=timeout,
            retry_on_timeout=retry_on_timeout,
        )

    def index_record(self, record: IndigoRecord):
        def gen():
            yield record

        return self.index_records(gen(), chunk_size=1)

    def index_records(self, records: Generator, chunk_size: int = 500):
        create_index(self.index_name, self.el_client)
        # pylint: disable=unused-variable
        for is_ok, action in streaming_bulk(
            self.el_client,
            prepare(self.index_type, records),
            index=self.index_name,
            chunk_size=chunk_size,
        ):
            pass

    def delete_all_records(self):
        try:
            self.el_client.indices.delete(index=self.index_name)
        except NotFoundError:
            pass

    def filter(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        indigo_session: Indigo = None,
        page_criteria: Optional[BingoElasticPageCriteria] = None,
        options: str = "",
        **kwargs,
    ) -> BingoElasticPageResult:
        # actions needed to be called on elastic_search result
        postprocess_actions: PostprocessType = []
        page_criteria = self.compile_query(
            query_subject=query_subject,
            page_criteria=page_criteria,
            postprocess_actions=postprocess_actions,
            **kwargs,
        )
        # We must NOT specify an index name as this is inherited by PIT.
        res = self.el_client.search(body=page_criteria.query)
        ret = get_page_result(
            res,
            self.index_type,
            page_criteria,
            postprocess_actions,
            indigo_session,
            options,
        )
        ret.synchronized()
        return ret

    def delete(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        limit: int = 1000,
        **kwargs,
    ) -> Dict[str, Any]:
        """
        Delete documents in index by a query filter.
        """
        if not self.el_client.indices.exists(index=self.index_name):
            return dict()
        page_criteria = self.compile_query(
            query_subject=query_subject,
            page_criteria=BingoElasticPageCriteria(page_size=limit - 1),
            is_delete_query=True,
            **kwargs,
        )
        return self.el_client.delete_by_query(
            index=self.index_name, body=page_criteria.query, slices="auto"
        )

    def compile_query(
        self,
        query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
        page_criteria: Optional[BingoElasticPageCriteria] = None,
        postprocess_actions: PostprocessType = None,
        is_delete_query: bool = False,
        **kwargs,
    ) -> BingoElasticPageCriteria:
        return _compile_query(
            self.index_name,
            self.el_client,
            query_subject,
            page_criteria,
            postprocess_actions,
            is_delete_query,
            **kwargs,
        )


def _compile_query(
    index_name: str,
    el_client: ElasticRepositoryT,
    query_subject: Union[BaseMatch, IndigoObject, IndigoRecord] = None,
    page_criteria: Optional[BingoElasticPageCriteria] = None,
    postprocess_actions: PostprocessType = None,
    is_delete_query: bool = False,
    **kwargs,
) -> BingoElasticPageCriteria:
    # record last elastic hit's sort object, regardless of its post-process filtering status.
    if postprocess_actions is None:
        postprocess_actions = []
    if page_criteria is None:
        page_criteria = BingoElasticPageCriteria()
    if not page_criteria.pit_id and not is_delete_query:
        pit_result = el_client.open_point_in_time(
            index=index_name,
            keep_alive=str(page_criteria.pit_stay_alive_minutes) + "m",
        )
        pit_id: str = pit_result["id"]
        page_criteria._pit_id = pit_id

    def page_processing_routine(
        record: IndigoRecord, indigo: Indigo, options: str
    ) -> Optional[IndigoRecord]:
        # This is the first post-processing action always, so it shouldn't return None
        assert record is not None
        page_criteria._next_page_search_after = record.sort
        return record

    if not is_delete_query:
        postprocess_actions.insert(0, page_processing_routine)

    query: Dict[str, Any]
    if not page_criteria.query:
        query = {
            "size": page_criteria.page_size + 1,
            "_source": {
                "includes": ["*"],
                "excludes": [
                    "sim_fingerprint",
                    "sim_fingerprint_len",
                    "sub_fingerprint_len",
                    "sub_fingerprint",
                ],
            },
            # Sort is necessary for paging.
            "sort": page_criteria.sort_criteria,
        }
        if not is_delete_query:
            query["pit"] = {
                "id": page_criteria.pit_id,
                "keep_alive": str(page_criteria.pit_stay_alive_minutes) + "m",
            }

        if isinstance(query_subject, BaseMatch):
            query_subject.compile(query, postprocess_actions)
        elif isinstance(query_subject, IndigoRecord):
            query_factory("exact", query_subject).compile(
                query, postprocess_actions
            )
        elif isinstance(query_subject, IndigoObject):
            query_subject.aromatize()
            query_factory("substructure", query_subject).compile(
                query, postprocess_actions
            )

        for key, value in kwargs.items():
            query_factory(key, value).compile(query)
    else:
        # We only bother to compile the query if this is the first page. Otherwise, we use the same query as before.
        query = page_criteria.query
    # But regardless of which page, we will overwrite search_after criteria if specified.
    if page_criteria.search_after:
        query["search_after"] = page_criteria.search_after
    page_criteria._query = query
    return page_criteria
