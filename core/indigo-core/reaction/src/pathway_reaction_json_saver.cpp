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

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "molecule/molecule.h"
#include "molecule/molecule_json_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/pathway_reaction_json_saver.h"

using namespace indigo;

IMPL_ERROR(PathwayReactionJsonSaver, "pathway reaction KET saver");

PathwayReactionJsonSaver::PathwayReactionJsonSaver(Output& output) : _output(output), add_stereo_desc(), pretty_json(), use_native_precision(false)
{
}

void PathwayReactionJsonSaver::saveReaction(PathwayReaction& pwr)
{
    auto merged = std::make_unique<Molecule>();
    for (int i = 0; i < pwr.getMoleculeCount(); ++i)
    {
        auto& molecule = pwr.getMolecule(i);
        merged->mergeWithMolecule(molecule, 0, 0);
    }
    merged->meta().clone(pwr.meta());
    rapidjson::StringBuffer buffer;
    JsonWriter writer(pretty_json);
    writer.Reset(buffer);
    MoleculeJsonSaver moleculeSaver(_output);
    moleculeSaver.add_stereo_desc = add_stereo_desc;
    moleculeSaver.use_native_precision = use_native_precision;
    moleculeSaver.saveMolecule(*merged, writer);
    _output.printf("%s", buffer.GetString());
}
