from pathlib import Path
from typing import Callable, Generator, Optional, Union

from indigo import Indigo, IndigoObject

from bingo_elastic.model.record import IndigoRecord


def iterate_file(
    file: Path,
    iterator: str = None,
    error_handler: Optional[Callable[[object, BaseException], None]] = None,
) -> Generator[IndigoRecord, None, None]:
    """
    :param file:
    :param iterator: supported iterators sdf, smiles, smi, cml.
                     If iterator is not set, trying to determine
                     iterator by file extension
    :type iterator: str
    :param error_handler: lambda for catching exceptions
    :type error_handler: Optional[Callable[[object, BaseException], None]]
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
        yield IndigoRecord(
            indigo_object=indigo_object, error_handler=error_handler
        )


def iterate_sdf(
    file: Union[Path, str],
    error_handler: Optional[Callable[[object, BaseException], None]] = None,
) -> Generator:
    yield from iterate_file(
        Path(file) if isinstance(file, str) else file,
        "sdf",
        error_handler=error_handler,
    )


def iterate_smiles(
    file: Union[Path, str],
    error_handler: Optional[Callable[[object, BaseException], None]] = None,
) -> Generator:
    yield from iterate_file(
        Path(file) if isinstance(file, str) else file,
        "smiles",
        error_handler=error_handler,
    )


def iterate_cml(
    file: Union[Path, str],
    error_handler: Optional[Callable[[object, BaseException], None]] = None,
) -> Generator:
    yield from iterate_file(
        Path(file) if isinstance(file, str) else file,
        "cml",
        error_handler=error_handler,
    )
