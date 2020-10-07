from typing import Union, List
import pathlib
import elasticsearch
import indigo
from dataclasses import dataclass


@dataclass(frozen=True)
class Record:

    finger_print: List[str]

    def __init__(self, mol: indigo.IndigoObject):
        # todo: remove hardcoded fp type
        self.__setattr__(self.finger_print,
                         mol.fingerprint('sim'))


def record_from_obj():
    pass


class ElasticDatabase:

    def __init__(self, elastic_url: str, index: str,
                 verify_certs: bool = False,
                 ssl_show_warn: bool = False):
        self.index = index
        self.session = indigo.Indigo()
        self.es = elasticsearch.Elasticsearch(
            [elastic_url],
            verify_certs=verify_certs,
            ssl_show_warn=ssl_show_warn,
        )

    def store(self, rec: Record) -> bool:
        self.es.create(
            {}
        )

    def exact_match(self, mol: indigo.IndigoObject) -> List[Record]:
        pass

    def similar_search(self, mol: indigo.IndigoObject, sim_type: str,
                       payload: float) -> List[Record]:
        pass


class BingoElastic:

    def __init__(self, elastic: ElasticDatabase):
        self.__elastic = elastic
        # todo: add mutex to multithreading
        self.__indigo_sess = indigo.Indigo()

    def save_mol(self, mol_src: Union[pathlib.Path, str]):
        if type(mol_src) == str:
            mol_src = pathlib.Path(mol_src)
        mol = self.__indigo_sess.loadQueryMolecule(mol_src)
        self.__elastic.store(mol)
