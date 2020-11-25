from pathlib import Path
from typing import Generator, Union

from indigo import Indigo, IndigoObject

from bingo_elastic.model.record import IndigoRecord


def iterate_file(
    file: Path, iterator: str = None
) -> Generator[IndigoRecord, None, None]:
    """
    :param file:
    :param iterator: supported iterators sdf, smiles, smi, cml.
                     If iterator is not set, trying to determine
                     iterator by file extension
    :type iterator: str
    :return:
    """
    iterators = {
        "sdf": "iterateSDFile",
        "smiles": "iterateSmilesFile",
        "smi": "iterateSmilesFile",
        "cml": "iterateCMLFile",
    }
    if not iterator:
        iterator = file.suffix[1:]
    iterator_fn = iterators.get(iterator)
    if not iterator_fn:
        raise AttributeError(f"Unsupported iterator {iterator}")

    indigo_object: IndigoObject
    for indigo_object in getattr(Indigo(), iterator_fn)(str(file)):
        yield IndigoRecord(indigo_object=indigo_object)


def iterate_sdf(file: Union[Path, str]) -> Generator:
    yield from iterate_file(Path(file) if type(file) == str else file, "sdf")


def iterate_smiles(file: Union[Path, str]) -> Generator:
    yield from iterate_file(
        Path(file) if type(file) == str else file, "smiles"
    )


def iterate_cml(file: Union[Path, str]) -> Generator:
    yield from iterate_file(Path(file) if type(file) == str else file, "cml")
