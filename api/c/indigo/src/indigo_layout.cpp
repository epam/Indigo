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

#include "base_cpp/cancellation_handler.h"
#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "layout/molecule_cleaner_2d.h"
#include "layout/molecule_layout.h"
#include "layout/reaction_layout.h"
#include "reaction/base_reaction.h"
#include <algorithm>
#include <vector>

CEXPORT int indigoLayout(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);
        int i;

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule* mol = &obj.getBaseMolecule();
            Filter f;
            if (obj.type == IndigoObject::SUBMOLECULE)
            {
                IndigoSubmolecule& submol = (IndigoSubmolecule&)obj;
                mol = &submol.getOriginalMolecule();
                f.initNone(mol->vertexEnd());
                for (int i = 0; i < submol.vertices.size(); i++)
                {
                    f.unhide(submol.vertices[i]);
                }
            }
            MoleculeLayout ml(*mol, self.smart_layout);

            if (obj.type == IndigoObject::SUBMOLECULE)
            {
                ml.filter = &f;
            }

            ml.max_iterations = self.layout_max_iterations;
            ml.bond_length = 1.6f;
            ml.layout_orientation = (layout_orientation_value)self.layout_orientation;
            bool has_atropisomery = mol->hasAtropisomericCenter();
            if (has_atropisomery)
                ml.respect_existing_layout = true;

            TimeoutCancellationHandler cancellation(self.cancellation_timeout);
            ml.setCancellationHandler(&cancellation);

            ml.make();

            if (obj.type != IndigoObject::SUBMOLECULE)
            {
                if (!has_atropisomery)
                    mol->clearBondDirections();
                try
                {
                    mol->markBondsStereocenters();
                    mol->markBondsAlleneStereo();
                }
                catch (Exception e)
                {
                }
                for (i = 1; i <= mol->rgroups.getRGroupCount(); i++)
                {
                    RGroup& rgp = mol->rgroups.getRGroup(i);

                    for (int j = rgp.fragments.begin(); j != rgp.fragments.end(); j = rgp.fragments.next(j))
                    {
                        rgp.fragments[j]->clearBondDirections();
                        try
                        {
                            rgp.fragments[j]->markBondsStereocenters();
                            rgp.fragments[j]->markBondsAlleneStereo();
                        }
                        catch (Exception e)
                        {
                        }
                    }
                }
            }
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            bool no_layout = rxn.intermediateCount() || rxn.specialConditionsCount() || rxn.meta().getNonChemicalMetaCount();
            if (!no_layout)
            {
                rxn.meta().resetMetaData();
                ReactionLayout rl(rxn, self.smart_layout);
                rl.max_iterations = self.layout_max_iterations;
                rl.layout_orientation = (layout_orientation_value)self.layout_orientation;
                rl.bond_length = 1.6f;
                rl.horizontal_interval_factor = self.layout_horintervalfactor;
                rl.make();
                try
                {
                    rxn.markStereocenterBonds();
                }
                catch (Exception e)
                {
                }
            }
        }
        else
        {
            throw IndigoError("The object provided is neither a molecule, nor a reaction");
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClean2d(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (IndigoBaseMolecule::is(obj))
        {
            if (obj.type == IndigoObject::SUBMOLECULE)
            {
                IndigoSubmolecule& submol = (IndigoSubmolecule&)obj;
                BaseMolecule& orig_mol = submol.getOriginalMolecule();

                std::vector<int> orig_vertices;
                for (int v : orig_mol.vertices())
                    orig_vertices.push_back(v);
                std::vector<int> submol_vertices;
                for (int i = 0; i < submol.vertices.size(); i++)
                    submol_vertices.push_back(submol.vertices[i]);
                std::sort(orig_vertices.begin(), orig_vertices.end());
                std::sort(submol_vertices.begin(), submol_vertices.end());
                bool is_same = orig_vertices.size() == submol_vertices.size();
                if (is_same)
                {
                    for (int i = 0; i < orig_vertices.size(); i++)
                        is_same &= orig_vertices[i] == submol_vertices[i];
                }
                if (is_same)
                {
                    MoleculeCleaner2d::clean(orig_mol);
                }
                else
                {
                    MoleculeCleaner2d cleaner2d1(orig_mol, false, submol.vertices);
                    cleaner2d1.do_clean(false);
                }
            }
            else
            {
                BaseMolecule& mol = obj.getBaseMolecule();
                MoleculeCleaner2d::clean(mol);
            }
        }
        else
        {
            if (IndigoBaseReaction::is(obj))
            {
                BaseReaction& rxn = obj.getBaseReaction();
                for (int i = rxn.begin(); i < rxn.end(); i = rxn.next(i))
                {
                    MoleculeCleaner2d::clean(rxn.getBaseMolecule(i));
                }
            }
            else
                throw IndigoError("Clean2d can be executed only for molecules but %s was provided", obj.debugInfo());
        }

        return 0;
    }
    INDIGO_END(-1);
}
