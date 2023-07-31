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

#include "base_cpp/array.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "layout/molecule_layout.h"
#include "layout/reaction_layout.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/rxnfile_saver.h"

#include "render_cdxml.h"
#include "render_context.h"
#include "render_grid.h"
#include "render_item_factory.h"
#include "render_item_molecule.h"
#include "render_params.h"
#include "render_single.h"

using namespace indigo;

RenderParams::RenderParams()
{
    clear();
}

RenderParams::~RenderParams()
{
}

void RenderParams::clearArrays()
{
    mols.clear();
    rxns.clear();
    titles.clear();
    refAtoms.clear();
}

void RenderParams::clear()
{
    relativeThickness = 1.0f;
    bondLineWidthFactor = 1.0f;
    rmode = RENDER_NONE;
    rOpt.clear();
    cnvOpt.clear();
    clearArrays();
}

IMPL_ERROR(RenderParamInterface, "render param interface");

bool RenderParamInterface::needsLayoutSub(BaseMolecule& mol)
{
    QS_DEF(RedBlackSet<int>, atomsToIgnore);
    atomsToIgnore.clear();
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
        {
            const Array<int>& atoms = sg.atoms;
            const Array<int>& patoms = ((MultipleGroup&)sg).parent_atoms;
            for (int j = 0; j < atoms.size(); ++j)
                atomsToIgnore.find_or_insert(atoms[j]);
            for (int j = 0; j < patoms.size(); ++j)
                if (atomsToIgnore.find(patoms[j]))
                    atomsToIgnore.remove(patoms[j]);
        }
    }
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (atomsToIgnore.find(i))
            continue;
        for (int j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j))
        {
            if (atomsToIgnore.find(j))
                continue;
            const Vec3f& v = mol.getAtomXyz(i);
            const Vec3f& w = mol.getAtomXyz(j);
            Vec3f d;
            d.diff(v, w);
            d.z = 0;
            if (d.length() < 1e-3)
                return true;
        }
    }
    return false;
}

bool RenderParamInterface::needsLayout(BaseMolecule& mol)
{
    if (needsLayoutSub(mol))
        return true;

    MoleculeRGroups& rGroups = mol.rgroups;
    for (int i = 1; i <= rGroups.getRGroupCount(); ++i)
    {
        PtrPool<BaseMolecule>& frags = rGroups.getRGroup(i).fragments;
        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
            if (needsLayoutSub(*frags[j]))
                return true;
    }
    return false;
}

void RenderParamInterface::_prepareMolecule(RenderParams& params, BaseMolecule& bm)
{
    if (needsLayout(bm))
    {
        MoleculeLayout ml(bm, params.smart_layout);
        ml.layout_orientation = UNCPECIFIED;
        ml.make();
        bm.clearBondDirections();
        bm.markBondsStereocenters();
        bm.markBondsAlleneStereo();
    }
}

void RenderParamInterface::_prepareReaction(RenderParams& params, BaseReaction& rxn)
{
    for (int i = rxn.begin(); i < rxn.end(); i = rxn.next(i))
    {
        BaseMolecule& mol = rxn.getBaseMolecule(i);
        if (needsLayout(mol))
        {
            MoleculeLayout ml(mol, params.smart_layout);
            ml.layout_orientation = UNCPECIFIED;
            ml.make();
            mol.clearBondDirections();
            mol.markBondsStereocenters();
            mol.markBondsAlleneStereo();
        }
    }
}

int RenderParamInterface::multilineTextUnit(RenderItemFactory& factory, int type, const Array<char>& titleStr, const float spacing,
                                            const MultilineTextLayout::Alignment alignment)
{
    int title = factory.addItemColumn();
    int start = 0;
    while (start < titleStr.size())
    {
        int next = titleStr.find(start + 1, titleStr.size(), '\n');
        if (next == -1)
            next = titleStr.size();
        int title_line = factory.addItemAuxiliary();
        factory.getItemAuxiliary(title_line).type = (RenderItemAuxiliary::AUX_TYPE)type;
        factory.getItemAuxiliary(title_line).text.copy(titleStr.ptr() + start, next - start);
        factory.getItemAuxiliary(title_line).text.push('\0');
        factory.getItemColumn(title).items.push(title_line);
        start = next + 1;
    }
    factory.getItemColumn(title).setVerticalSpacing(spacing);
    factory.getItemColumn(title).setAlignment(alignment);
    return title;
}

void RenderParamInterface::render(RenderParams& params)
{
    if (params.rmode == RENDER_NONE)
        throw Error("No object to render specified");

    RenderContext rc(params.rOpt, params.relativeThickness, params.bondLineWidthFactor);

    bool bondLengthSet = params.cnvOpt.bondLength > 0;
    int bondLength = (int)(bondLengthSet ? params.cnvOpt.bondLength : 100);
    rc.setDefaultScale((float)bondLength); // TODO: fix bondLength type

    RenderItemFactory factory(rc);
    int obj = -1;
    Array<int> objs;
    Array<int> titles;
    if (params.rmode == RENDER_MOL)
    {
        if (params.mols.size() == 0)
        {
            obj = factory.addItemMolecule();
            BaseMolecule& bm = *params.mol;
            _prepareMolecule(params, bm);
            factory.getItemMolecule(obj).mol = &bm;
        }
        else
        {
            for (int i = 0; i < params.mols.size(); ++i)
            {
                int mol = factory.addItemMolecule();
                BaseMolecule& bm = *params.mols[i];
                _prepareMolecule(params, bm);
                factory.getItemMolecule(mol).mol = &bm;
                objs.push(mol);

                if (params.titles.size() > 0)
                {
                    titles.push(multilineTextUnit(factory, RenderItemAuxiliary::AUX_TITLE, params.titles[i],
                                                  params.rOpt.titleSpacing * params.rOpt.titleFontFactor, params.cnvOpt.titleAlign.inbox_alignment));
                }
            }
        }
    }
    else if (params.rmode == RENDER_RXN)
    {
        if (params.rxns.size() == 0)
        {
            obj = factory.addItemReaction();
            factory.getItemReaction(obj);
            BaseReaction& rxn = *params.rxn;
            _prepareReaction(params, rxn);
            factory.getItemReaction(obj).rxn = &rxn;
        }
        else
        {
            for (int i = 0; i < params.rxns.size(); ++i)
            {
                int rxn = factory.addItemReaction();
                BaseReaction& br = *params.rxns[i];
                _prepareReaction(params, br);
                factory.getItemReaction(rxn).rxn = &br;
                objs.push(rxn);

                if (params.titles.size() > 0)
                {
                    titles.push(multilineTextUnit(factory, RenderItemAuxiliary::AUX_TITLE, params.titles[i],
                                                  params.rOpt.titleSpacing * params.rOpt.titleFontFactor, params.cnvOpt.titleAlign.inbox_alignment));
                }
            }
        }
    }
    else
    {
        throw Error("Invalid rendering mode: %i", params.rmode);
    }

    int comment = -1;
    if (params.cnvOpt.comment.size() > 0)
    {
        comment = multilineTextUnit(factory, RenderItemAuxiliary::AUX_COMMENT, params.cnvOpt.comment,
                                    params.rOpt.commentSpacing * params.rOpt.commentFontFactor, params.cnvOpt.commentAlign.inbox_alignment);
    }

    // Render into other formats after objects has been prepared (layout, etc.)
    if (params.rOpt.mode == MODE_CDXML)
    {
        // Render into CDXML format
        RenderParamCdxmlInterface::render(params);
        return;
    }

    if (obj >= 0)
    {
        RenderSingle render(rc, factory, params.cnvOpt, bondLength, bondLengthSet);
        render.obj = obj;
        render.comment = comment;
        render.draw();
    }
    else
    {
        RenderGrid render(rc, factory, params.cnvOpt, bondLength, bondLengthSet);
        render.objs.copy(objs);
        render.comment = comment;
        render.titles.copy(titles);
        render.refAtoms.copy(params.refAtoms);
        render.draw();
    }
    rc.closeContext(false);
}
