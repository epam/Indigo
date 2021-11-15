#
# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from typing import List, Optional, Tuple, Union

from indigo import IndigoObject
from indigo.inchi import IndigoInchi

from indigo_service import jsonapi
from indigo_service.indigo_tools import indigo


def extract_compounds(
    pairs: List[Tuple[str, jsonapi.CompoundFormat]],
    modifiers: Optional[List[jsonapi.CompoundModifiers]] = None,
) -> List[IndigoObject]:
    result = []
    for compound, compound_type in pairs:
        if compound_type == jsonapi.CompoundFormat.AUTO:
            indigo_object = indigo().loadMolecule(compound)
        elif compound_type == jsonapi.CompoundFormat.SMARTS:
            indigo_object = indigo().loadSmarts(compound)
        else:
            raise RuntimeError(f"{compound_type=} is not supported")
        if modifiers:
            for mod in modifiers:
                getattr(indigo_object, mod)()
        result.append(indigo_object)
    return result


def extract_pairs(
    compounds: Union[jsonapi.CompoundObject, List[jsonapi.CompoundObject]]
) -> List[Tuple[str, jsonapi.CompoundFormat]]:
    if isinstance(compounds, list):
        return [(comp.structure, comp.format) for comp in compounds]
    return [(compounds.structure, compounds.format)]


def to_string(
    compound: IndigoObject, string_format: jsonapi.CompoundFormat
) -> Tuple[str, jsonapi.CompoundFormat]:

    if string_format == jsonapi.CompoundFormat.AUTO:
        string_format = jsonapi.CompoundFormat.MOLFILE

    if string_format == jsonapi.CompoundFormat.SMILES:
        return compound.smiles(), string_format
    if string_format == jsonapi.CompoundFormat.MOLFILE:
        return compound.molfile(), string_format
    if string_format == jsonapi.CompoundFormat.CML:
        return compound.cml(), string_format
    if string_format == jsonapi.CompoundFormat.SMARTS:
        return compound.smarts(), string_format
    if string_format == jsonapi.CompoundFormat.INCHI:
        return IndigoInchi(indigo()).getInchi(compound), string_format
    raise RuntimeError(f"{string_format} is not supported")


def map_match_output(
    match: Optional[IndigoObject],
    output_format: jsonapi.MatchOutputFormat,
    target: IndigoObject,
) -> Union[str, jsonapi.MapBondModel, jsonapi.MapAtomModel]:
    if match is None:
        raise AttributeError("Empty result")
    if output_format == jsonapi.MatchOutputFormat.HIGHLIGHTED_TARGET_MOLFILE:
        return match.highlightedTarget().molfile()  # type: ignore
    if output_format == jsonapi.MatchOutputFormat.HIGHLIGHTED_TARGET_SMILES:
        return match.highlightedTarget().smiles()  # type: ignore
    if output_format == jsonapi.MatchOutputFormat.HIGHLIGHTED_TARGET_CML:
        return match.highlightedTarget().cml()  # type: ignore
    if output_format == jsonapi.MatchOutputFormat.MAP_ATOM:
        return map_atom(match, target)
    if output_format == jsonapi.MatchOutputFormat.MAP_BOND:
        return map_bond(match, target)
    raise RuntimeError(f"{output_format=} is not supported yet")


def map_bond(
    match: IndigoObject, compound: IndigoObject
) -> jsonapi.MapBondModel:
    source = []
    target = []
    for source_index, bond in enumerate(compound.iterateBonds()):
        if match.mapBond(bond):
            source.append(source_index)
            target.append(bond.index())
    return jsonapi.MapBondModel(sourceBonds=source, targetBonds=target)


def map_atom(
    match: IndigoObject, compound: IndigoObject
) -> jsonapi.MapAtomModel:
    source = []
    target = []
    for source_index, atom in enumerate(compound.iterateAtoms()):
        if match.mapAtom(atom):
            source.append(source_index)
            target.append(atom.index())
    return jsonapi.MapAtomModel(sourceAtoms=source, targetAtoms=target)


def upperfirst(text: str) -> str:
    if len(text) > 1:
        return text[0].capitalize() + text[1:]
    return text.capitalize()


def validate(compound: IndigoObject, validation: jsonapi.Validations) -> str:
    if validation == jsonapi.Validations.STEREO_3D:
        return str(compound.check3DStereo())
    check_method = "check" + upperfirst(validation.value)
    return str(getattr(compound, check_method)())


def get_descriptor(
    compound: IndigoObject, descriptor: jsonapi.Descriptors
) -> str:
    return str(getattr(compound, descriptor.NAME)())
