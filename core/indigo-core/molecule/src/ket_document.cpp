/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "molecule/ket_document.h"
#include "base_cpp/exception.h"
#include "molecule/base_molecule.h"

using namespace indigo;

IMPL_ERROR(KetDocument, "Ket Document")

KetMolecule& KetDocument::addMolecule(const std::string& ref)
{
    _molecule_refs.emplace_back(ref);
    auto res = _molecules.emplace(ref, KetMolecule());
    if (!res.second)
        throw Error("Molecule with ref='%s' already exists", ref.c_str());
    return res.first->second;
}

KetMonomer& KetDocument::addMonomer(const std::string& ref, const std::string& id, const std::string& alias, const std::string& template_id)
{
    _monomers_refs.emplace_back(ref);
    auto res = _monomers.emplace(ref, KetMonomer(id, alias, template_id));
    if (!res.second)
        throw Error("Monomer with ref='%s' already exists", ref.c_str());
    return res.first->second;
}

KetMonomer& KetDocument::addMonomer(const std::string& alias, const std::string& template_id)
{
    std::string id = std::to_string(_monomers.size());
    return addMonomer("monomer" + id, id, alias, template_id);
}

void KetDocument::addMonomerTemplate(const MonomerTemplate& monomer_template)
{
    std::string ref = MonomerTemplate::ref_prefix + monomer_template.id();
    _templates_refs.emplace_back(ref);
    auto it = _templates.try_emplace(ref, monomer_template.id(), monomer_template.monomerClass(), monomer_template.idtAlias(), monomer_template.unresolved());
    if (!it.second)
        throw Error("Template with id '%s' already added", monomer_template.id().c_str());
    it.first->second.copy(monomer_template);
}

BaseMolecule& KetDocument::getBaseMolecule()
{
    if (!_molecule.has_value())
    {
        // save to ket
        // load molecule from ket
    }
    return *_molecule.value().get();
}
