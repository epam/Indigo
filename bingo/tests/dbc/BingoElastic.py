import time

from bingo_elastic.elastic import ElasticRepository
from bingo_elastic.model.record import (
    IndigoRecord,
    IndigoRecordMolecule,
    IndigoRecordReaction,
)
from bingo_elastic.queries import SimilarityMatch
from indigo import IndigoException, IndigoObject

from ..constants import DB_BINGO_ELASTIC, EntitiesType
from ..helpers import indigo_iterator
from ..logger import logger
from .base import NoSQLAdapter, catch_indigo_exception


class BingoElastic(NoSQLAdapter):
    dbms = DB_BINGO_ELASTIC
    host = None
    port = None
    repo = None

    def __init__(self, indigo, index_name):
        NoSQLAdapter.__init__(self)
        self.indigo = indigo
        logger.info(f"Connecting to {self.dbms} repository")
        self.repo = ElasticRepository(
            host=self.host, port=self.port, index_name=index_name
        )

    def _set_db_config(self):
        self.host = self.config[self.dbms]["host"]
        self.port = self.config[self.dbms]["port"]

    def import_data(self, data_path: str, database_type: str):
        logger.info(f"Importing data to {self.dbms} from {data_path}")
        index = 1
        for compound in indigo_iterator(self.indigo, data_path):
            try:
                if database_type == EntitiesType.MOLECULES:
                    record = IndigoRecordMolecule(
                        indigo_object=compound, skip_errors=True, index=index
                    )
                else:
                    record = IndigoRecordReaction(
                        indigo_object=compound, skip_errors=True, index=index
                    )
                self.repo.index_record(record)
            except IndigoException as e:
                logger.error(
                    f"Error during import {database_type} from "
                    f"{data_path} (id = {index}) "
                    f"'{compound.rawData()[:20]}...': {e}"
                )
            finally:
                index += 1
        time.sleep(1)

    @catch_indigo_exception(catch_error=True)
    def exact(
        self, molecule: IndigoObject, target_function: str, options: str = ""
    ):
        compound = self.indigo.loadMolecule(molecule.rawData())
        indigo_record = IndigoRecord(indigo_object=compound)
        records = self.repo.filter(
            query_subject=indigo_record,
            indigo_session=self.indigo,
            limit=5000,
            options=options,
        )

        return self._process_records(records)

    @catch_indigo_exception(catch_error=True)
    def substructure(
        self, molecule: IndigoObject, target_function: str, options: str = ""
    ):
        query_mol = self.indigo.loadQueryMolecule(molecule.rawData())
        query_mol.aromatize()
        records = self.repo.filter(
            query_subject=query_mol,
            indigo_session=query_mol.dispatcher,
            limit=5000,
            options=options,
        )
        return self._process_records(records)

    @catch_indigo_exception(catch_error=True)
    def similarity(
        self, molecule: IndigoObject, target_function: str, sim_type, options: str
    ):
        min_sim, max_sim = options.split(", ")
        min_sim, max_sim = float(min_sim), float(max_sim)
        if sim_type.startswith("euclid"):
            sim_type = "euclid"
        compound = self.indigo.loadMolecule(molecule.rawData())
        indigo_record = IndigoRecord(indigo_object=compound)
        alg = getattr(SimilarityMatch, sim_type)(indigo_record, min_sim)
        records = self.repo.filter(query_subject=alg, limit=5000)

        return self._process_records(records)

    def _process_records(self, records):
        result = []
        for record in records:
            if record:
                result.append(record.as_dict()["index"])

        return result

    def drop(self):
        logger.info(f"Dropping {self.dbms} repository")
        self.repo.delete_all_records()
