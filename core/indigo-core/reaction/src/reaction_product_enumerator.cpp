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

#include "base_cpp/output.h"

#include "reaction/reaction_enumerator_state.h"
#include "reaction/reaction_product_enumerator.h"

#include "base_c/bitarray.h"
#include "base_cpp/gray_codes.h"
#include "base_cpp/tlscont.h"
#include "graph/dfs_walk.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph.h"
#include "graph/spanning_tree.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(ReactionProductEnumerator, "Reaction product enumerator");

CP_DEF(ReactionProductEnumerator);

ReactionProductEnumerator::ReactionProductEnumerator(QueryReaction& reaction)
    : is_multistep_reaction(false), is_self_react(false), is_one_tube(false), max_product_count(1000), max_deep_level(2), _reaction(reaction), CP_INIT,
      TL_CP_GET(_product_aam_array), TL_CP_GET(_smiles_array), TL_CP_GET(_tubes_monomers)
{
    _product_aam_array.clear();
    _smiles_array.clear();
    _tubes_monomers.clear();
    _product_count = 0;
    _is_rg_exist = false;
    refine_proc = 0;
    product_proc = 0;
}

void ReactionProductEnumerator::addMonomer(int reactant_idx, Molecule& monomer)
{
    _reaction_monomers.addMonomer(reactant_idx, monomer);
}

void ReactionProductEnumerator::clearReactantMonomers(int reactant_idx)
{
    QS_DEF(Array<int>, unused_monomers);
    unused_monomers.clear();

    for (int i = _reaction_monomers._monomers.size() - 1; i >= 0; i--)
        if (_reaction_monomers._reactant_indexes[i] == reactant_idx)
            _reaction_monomers.removeMonomer(i);
}

Molecule& ReactionProductEnumerator::getMonomer(int reactant_idx, int index)
{
    return _reaction_monomers.getMonomer(reactant_idx, index);
}

Molecule& ReactionProductEnumerator::getMonomer(int mon_index)
{
    return _reaction_monomers.getMonomer(mon_index);
}

const QueryReaction& ReactionProductEnumerator::getReaction(void)
{
    return _reaction;
}

int ReactionProductEnumerator::getMonomersCount(int reactant_idx)
{
    int monomers_count = 0;

    for (int i = 0; i < _reaction_monomers._reactant_indexes.size(); i++)
        if (_reaction_monomers._reactant_indexes[i] == reactant_idx)
            monomers_count++;

    return monomers_count;
}

void ReactionProductEnumerator::buildProducts(void)
{
    QS_DEF(QueryMolecule, all_products);
    all_products.clear();

    for (int i = 0; i < _reaction_monomers.size(); i++)
    {
        if (_reaction_monomers._deep_levels[i] > 0)
        {
            _reaction_monomers.removeMonomer(i);
            i--;
        }
    }

    /* Building of monomer tubes grid */
    if (!is_one_tube)
        _buildTubesGrid();

    for (int i = _reaction.productBegin(); i != _reaction.productEnd(); i = _reaction.productNext(i))
    {
        QueryMolecule& product = _reaction.getQueryMolecule(i);
        QS_DEF(Array<int>, mapping);
        mapping.clear();

        all_products.mergeWithMolecule(product, &mapping);
        _product_aam_array.expand(all_products.vertexEnd());
        for (int j = product.vertexBegin(); j != product.vertexEnd(); j = product.vertexNext(j))
            _product_aam_array[mapping[j]] = _reaction.getAAM(i, j);
    }

    all_products.buildCisTrans(NULL);

    _smiles_array.clear();
    _product_count = 0;

    ReactionEnumeratorContext context;
    context.arom_options = arom_options;

    ReactionEnumeratorState rpe_state(context, _reaction, all_products, _product_aam_array, _smiles_array, _reaction_monomers, _product_count, _tubes_monomers);

    rpe_state.refine_proc = refine_proc;
    rpe_state.product_proc = product_proc;
    rpe_state.userdata = userdata;
    rpe_state.is_multistep_reaction = is_multistep_reaction;
    rpe_state.is_self_react = is_self_react;
    rpe_state.max_deep_level = max_deep_level;
    rpe_state.max_product_count = max_product_count;
    rpe_state.is_one_tube = is_one_tube;

    rpe_state.buildProduct();
}

void ReactionProductEnumerator::_buildTubesGrid(void)
{
    QS_DEF(ObjArray<Array<int>>, digits);
    digits.clear();

    int digit_idx = 0;
    int val = 0;
    for (int i = _reaction.reactantBegin(); i != _reaction.reactantEnd(); i = _reaction.reactantNext(i))
    {
        int monomers_count = getMonomersCount(i);
        Array<int>& new_array = digits.push();

        for (int j = 0; j < monomers_count; j++)
            new_array.push(val++);

        digit_idx++;
    }

    int tubes_count = 1;
    for (int i = _reaction.reactantBegin(); i != _reaction.reactantEnd(); i = _reaction.reactantNext(i))
        tubes_count *= getMonomersCount(i);
    _tubes_monomers.resize(tubes_count);

    for (int i = 0; i < tubes_count; i++)
    {
        int cur_tube_code = i;
        int code_pos = 0;
        int dev = 1;
        for (int j = _reaction.reactantBegin(); j != _reaction.reactantEnd(); j = _reaction.reactantNext(j))
        {
            int monomers_count = getMonomersCount(j);

            val = cur_tube_code % monomers_count;
            cur_tube_code /= monomers_count;

            _tubes_monomers[i].push(digits[code_pos][val]);

            dev *= getMonomersCount(j);
            code_pos++;
        }
    }
}