from pathlib import Path
from typing import Callable, Generator, Optional, Union

from indigo import Indigo, IndigoObject  # type: ignore

from bingo_elastic.model.record import (
    IndigoRecordMolecule,
    IndigoRecordReaction,
)


def iterate_file(
    file: Path,
    iterator: Optional[str] = None,
    error_handler: Optional[Callable[[object, BaseException], None]] = None,
) -> Generator[IndigoRecordMolecule, None, None]:
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
        yield IndigoRecordMolecule(
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


def load_molecule(
    file_: Union[str, Path], session: Indigo
) -> IndigoRecordMolecule:
    """
    Helper for loading molecules from file into IndigoRecordMolecule object
    """
    molecule = session.loadMoleculeFromFile(file_)
    return IndigoRecordMolecule(indigo_object=molecule)


def load_reaction(
    file_: Union[str, Path], session: Indigo
) -> IndigoRecordReaction:
    """
    Helper for loading reactions into IndigoRecordReaction object
    """
    reaction = session.loadReactionFromFile(str(file_))
    return IndigoRecordReaction(indigo_object=reaction)
