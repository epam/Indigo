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

#include "bingo_context.h"
#include "base_cpp/scanner.h"
#include "bingo_error.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_loader.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_saver.h"

// const char * bingo_version_string = "1.7-beta3";

using namespace indigo;

TL_DEF(BingoContext, PtrArray<BingoContext>, _instances);

std::mutex BingoContext::_instances_lock;

IMPL_ERROR(BingoContext, "bingo context");

BingoContext::BingoContext(int id_)
{
    id = id_;
    reset();
    treat_x_as_pseudoatom.setName("treat-x-as-pseudoatom");
    ignore_closing_bond_direction_mismatch.setName("ignore-closing-bond-direction-mismatch");
    ignore_stereocenter_errors.setName("ignore-stereocenter-errors");
    stereochemistry_bidirectional_mode.setName("stereochemistry-bidirectional-mode");
    stereochemistry_detect_haworth_projection.setName("stereochemistry-detect-haworth-projection");
    ignore_cistrans_errors.setName("ignore-cistrans-errors");
    allow_non_unique_dearomatization.setName("allow-non-unique-dearomatization");
    zero_unknown_aromatic_hydrogens.setName("zero-unknown-aromatic-hydrogens");
    reject_invalid_structures.setName("reject-invalid-structures");
    ignore_bad_valence.setName("ignore-bad-valence");
    ct_format_save_date.setName("ct_format_save_date");
}

void BingoContext::reset()
{
    cmf_dict.reset();

    nthreads = 0;
    timeout = DEFAULT_TIMEOUT;

    tautomer_rules_ready = false;
    fp_parameters_ready = false;
    atomic_mass_map_ready = false;

    treat_x_as_pseudoatom.reset();
    ignore_closing_bond_direction_mismatch.reset();
    ignore_stereocenter_errors.reset();
    stereochemistry_bidirectional_mode.reset();
    stereochemistry_detect_haworth_projection.reset();
    ignore_cistrans_errors.reset();
    allow_non_unique_dearomatization.reset();
    zero_unknown_aromatic_hydrogens.reset();
    reject_invalid_structures.reset();
    ignore_bad_valence.reset();
    ct_format_save_date.reset();
    ct_format_mode = 0;
}

BingoContext::~BingoContext()
{
}

void BingoContext::remove(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<BingoContext>, _instances);
    int i;

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            break;

    // if (i == _instances.size())
    //   throw Error("remove(): context #%d not found", id);

    if (i != _instances.size())
        _instances.remove(i);
}

BingoContext* BingoContext::_get(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<BingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            return _instances[i];

    return 0;
}

BingoContext* BingoContext::existing(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<BingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            return _instances[i];

    throw Error("context #%d not found", id);
}

BingoContext* BingoContext::get(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<BingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            return _instances[i];

    return &_instances.add(new BingoContext(id));
}

void bingoGetTauCondition(const char* list_ptr, int& aromaticity, Array<int>& label_list)
{
    if (isdigit(*list_ptr))
    {
        if (*list_ptr == '1')
            aromaticity = 1;
        else if (*list_ptr == '0')
            aromaticity = 0;
        else
            throw BingoError("Bad tautomer configuration string format");
        list_ptr++;
    }
    else
    {
        aromaticity = -1;
    }

    label_list.clear();

    QS_DEF(Array<char>, buf);
    buf.clear();

    while (*list_ptr != 0)
    {
        if (isalpha((unsigned)*list_ptr))
            buf.push(*list_ptr);
        else if (*list_ptr == ',')
        {
            buf.push(0);
            label_list.push(Element::fromString(buf.ptr()));
            buf.clear();
        }
        else
            throw BingoError("Bad label list format");
        list_ptr++;
    }

    buf.push(0);
    label_list.push(Element::fromString(buf.ptr()));
}

void bingoGetName(Scanner& scanner, Array<char>& result)
{
    QS_DEF(Array<char>, str);
    bool single_line = Scanner::isSingleLine(scanner);

    result.clear();

    if (single_line)
    {
        scanner.readLine(str, true);
        int idx = str.find('|');
        int idx2 = 0;

        if (idx >= 0)
        {
            int tmp = str.find(idx + 1, str.size(), '|');

            if (tmp > 0)
                idx2 = tmp + 1;
        }

        BufferScanner scan2(str.ptr() + idx2);

        while (!scan2.isEOF())
        {
            if (isspace(scan2.readChar()))
                break;
        }
        scan2.skipSpace();
        if (!scan2.isEOF())
            scan2.readLine(result, false);
    }
    else
    {
        scanner.readLine(result, false);
        if (result.size() >= 4 && strncmp(result.ptr(), "$RXN", 4) == 0)
            scanner.readLine(result, false);
    }
}

StereocentersOptions BingoContext::getStereocentersOptions()
{
    StereocentersOptions opt;
    opt.ignore_errors = ignore_stereocenter_errors;
    opt.bidirectional_mode = stereochemistry_bidirectional_mode;
    opt.detect_haworth_projection = stereochemistry_detect_haworth_projection;
    return opt;
}

void BingoContext::setLoaderSettings(MoleculeAutoLoader& loader)
{
    loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
    loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    loader.stereochemistry_options = getStereocentersOptions();
    loader.ignore_cistrans_errors = ignore_cistrans_errors;
    loader.ignore_bad_valence = ignore_bad_valence;
}

void BingoContext::setLoaderSettings(SmilesLoader& loader)
{
    loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    loader.stereochemistry_options = getStereocentersOptions();
    loader.ignore_cistrans_errors = ignore_cistrans_errors;
    loader.ignore_bad_valence = ignore_bad_valence;
}

void BingoContext::setLoaderSettings(ReactionAutoLoader& loader)
{
    loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
    loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    loader.stereochemistry_options = getStereocentersOptions();
    loader.ignore_cistrans_errors = ignore_cistrans_errors;
    loader.ignore_bad_valence = ignore_bad_valence;
}

void BingoContext::setLoaderSettings(RSmilesLoader& loader)
{
    loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
    loader.stereochemistry_options = getStereocentersOptions();
    loader.ignore_cistrans_errors = ignore_cistrans_errors;
    loader.ignore_bad_valence = ignore_bad_valence;
}

void BingoContext::setSaverSettings(MolfileSaver& saver)
{
    saver.mode = ct_format_mode;
    saver.skip_date = !ct_format_save_date;
}

void BingoContext::setSaverSettings(RxnfileSaver& saver)
{
    saver.molfile_saving_mode = ct_format_mode;
    saver.skip_date = !ct_format_save_date;
}
