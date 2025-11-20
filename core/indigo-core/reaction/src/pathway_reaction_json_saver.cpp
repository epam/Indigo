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

#include <memory>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "layout/molecule_layout.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/pathway_reaction_builder.h"
#include "reaction/pathway_reaction_json_saver.h"
#include "reaction/reaction_multistep_detector.h"

using namespace indigo;

IMPL_ERROR(PathwayReactionJsonSaver, "pathway reaction KET saver");

PathwayReactionJsonSaver::PathwayReactionJsonSaver(Output& output)
    : _output(output), add_stereo_desc(), pretty_json(), use_native_precision(false), add_reaction_data(false)
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
    indigo::LayoutOptions layout_options;
    ReactionMultistepDetector rmd(*merged, layout_options);
    if (add_reaction_data)
    {
        rmd.detectReaction();
        rmd.buildReactionsData();
    }
    rapidjson::StringBuffer buffer;
    auto writer_ptr = JsonWriter::createJsonWriter(pretty_json);
    JsonWriter& writer = *writer_ptr;
    writer.Reset(buffer);
    MoleculeJsonSaver moleculeSaver(_output, rmd);
    moleculeSaver.add_stereo_desc = add_stereo_desc;
    moleculeSaver.use_native_precision = use_native_precision;
    moleculeSaver.add_reaction_data = add_reaction_data;
    moleculeSaver.saveMolecule(*merged, writer);
    _output.printf("%s", buffer.GetString());
}
