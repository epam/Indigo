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

#ifndef __loader_options_h__
#define __loader_options_h__

#include "molecule/molecule_stereocenter_options.h"
#include "molecule/valence_model.h"

namespace indigo
{
    // Shared options POD propagated from Indigo C-API into every concrete loader
    // (MolfileLoader, CmlLoader, SmilesLoader, MoleculeAutoLoader, MoleculeCdxmlLoader,
    // and the reaction loaders that wrap them).
    //
    // Closes the Shotgun Surgery + Insider Trading smells: adding a new option now
    // touches this single struct and every loader picks it up via setOptions().
    // Loaders that do not consume a particular field (e.g. SmilesLoader has no
    // 3D chirality) simply ignore it.
    //
    // Defaults must match master behaviour — keep `valence_mode = BIOVIA_2009`
    // until BIOVIA 2017 is opt-in across the wrapper layer.
    struct LoaderOptions
    {
        StereocentersOptions stereochemistry_options;
        ValenceMode valence_mode = ValenceMode::BIOVIA_2009;
        bool ignore_bad_valence = false;
        bool ignore_no_chiral_flag = false;
        bool ignore_noncritical_query_features = false;
        bool skip_3d_chirality = false;
        bool treat_x_as_pseudoatom = false;
    };

} // namespace indigo

#endif
