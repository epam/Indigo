from os import listdir, remove, rmdir
from os.path import join

from indigo import IndigoObject
from indigo.bingo import Bingo, BingoException

from ..constants import DB_BINGO
from ..helpers import indigo_iterator
from ..logger import logger
from .base import NoSQLAdapter, catch_indigo_exception


class BingoNoSQL(NoSQLAdapter):
    dbms = DB_BINGO

    def __init__(self, indigo):
        NoSQLAdapter.__init__(self)
        self.indigo = indigo

    def connect(self):
        logger.info(f"Connecting to {self.dbms} DB")
        self.bingo = Bingo.loadDatabaseFile(self.indigo, self.db_path)

    def close_connect(self):
        logger.info(f"Closing connecting to {self.dbms} DB")
        self.bingo.close()

    def import_data(self, data_path: str, database_type: str):
        logger.info(f"Creating {self.dbms} database")
        self.bingo = Bingo.createDatabaseFile(
            self.indigo, self.db_path, database_type
        )

        logger.info(f"Importing data to {self.dbms} from {data_path}")
        index = 1
        for mol in indigo_iterator(self.indigo, data_path):
            try:
                self.bingo.insert(mol, index)
            except BingoException as e:
                logger.error(
                    f"Error during import {database_type} from "
                    f"{data_path} (id = {index}) "
                    f"'{mol.rawData()[:20]}...': {e}"
                )
            finally:
                index += 1
        self.close_connect()

    def delete_base(self):
        logger.info(f"Dropping {self.dbms} database")
        for db_file in listdir(join(self.db_dir, self.db_name)):
            remove(join(self.db_dir, self.db_name, db_file))
        rmdir(join(self.db_dir, self.db_name))

    @catch_indigo_exception()
    def mass(self, molecule: IndigoObject, weight_type: str):
        if weight_type == "molecular-weight":
            return molecule.molecularWeight()
        if weight_type == "most-abundant-mass":
            return molecule.mostAbundantMass()
        if weight_type == "monoisotopic-mass":
            return molecule.monoisotopicMass()

    @catch_indigo_exception()
    def gross(self, molecule: IndigoObject):
        return molecule.grossFormula()

    @catch_indigo_exception(catch_error=True)
    def exact(self, molecule, target_function=None, options=""):
        result = []
        exact_matcher = self.bingo.searchExact(molecule, options)
        while exact_matcher.next():
            id = exact_matcher.getCurrentId()
            result.append(id)
        exact_matcher.close()
        return result

    @catch_indigo_exception(catch_error=True)
    def substructure(self, molecule, target_function=None, options=""):
        result = []
        query = self.indigo.loadQueryMolecule(molecule.rawData())
        sub_matcher = self.bingo.searchSub(query, options)
        while sub_matcher.next():
            id = sub_matcher.getCurrentId()
            result.append(id)
        sub_matcher.close()
        return result

    @catch_indigo_exception(catch_error=True)
    def similarity(self, molecule, target_function=None, options=""):
        result = []
        sim_type, min_sim, max_sim = options.split(", ")
        min_sim, max_sim = float(min_sim), float(max_sim)
        sim_matcher = self.bingo.searchSim(
            molecule, min_sim, max_sim, sim_type
        )
        while sim_matcher.next():
            id = sim_matcher.getCurrentId()
            result.append(id)
        sim_matcher.close()
        return result

    @catch_indigo_exception(catch_error=True)
    def smarts(self, molecule, target_function=None, options=""):
        return self.substructure(molecule, target_function, options)

    @catch_indigo_exception(catch_error=True)
    def rsmarts(self, reaction, target_function=None, options=""):
        return self.substructure(reaction, target_function, options)

    @catch_indigo_exception(catch_error=True)
    def rexact(self, reaction, target_function=None, options=""):
        result = []
        exact_matcher = self.bingo.searchExact(reaction, options)
        while exact_matcher.next():
            id = exact_matcher.getCurrentId()
            result.append(id)
        exact_matcher.close()
        return result

    @catch_indigo_exception(catch_error=True)
    def rsubstructure(self, reaction, target_function=None, options=""):
        result = []
        query = self.indigo.loadQueryReaction(reaction.rawData())
        sub_matcher = self.bingo.searchSub(query, options)
        while sub_matcher.next():
            id = sub_matcher.getCurrentId()
            result.append(id)
        sub_matcher.close()
        return result
