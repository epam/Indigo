/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "base_cpp/array.h"
#include "molecule/base_molecule.h"

#include "molecule/molecule_rgroups_composition.h"

using namespace indigo;

IMPL_ERROR(MoleculeRGroupsComposition, "molecule rgroups composition");

Iterable<Attachment*>* MoleculeRGroupsComposition::refine(BaseMolecule &mol) {
    MoleculeRGroups &rgroups = mol.rgroups;
    auto k = rgroups.getRGroupCount();
    auto n = mol.countRSites();

    MultiMap site2group;
    MultiMap group2site;
    Map      group2size;
    for (auto i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i)) {
        if (!mol.isRSite(i)) { continue; }

        int site = i + 1;
        Array<int> groups;
        mol.getAllowedRGroups(i, groups);
        site2group.insert(site, groups);
        for (auto j = 0; j < groups.size(); j++) {
            group2site.insert(groups[j], site);
        }
    }

    Topology top(group2site.size());
    OccurrenceRestrictions occurrences(k);
    for (auto group = 1; group <= k; group++) {
        const RGroup &rg = rgroups.getRGroup(group);
        if (rg.rest_h == 1) {
            const Set &sites = group2site[group];
            for (auto i = sites.begin(); i != sites.end(); i += sites.next(i)) {
                auto site = sites.key(i);
                site2group.remove(site);
                site2group.insert(site, group);
            }
        }

        group2size.insert(group, rg.fragments.size());
        if (rg.if_then) {
            top.depends(group, rg.if_then);
        }
        occurrences.set(group, convert(rg.occurrence));
    }

    return new Attachments(n, k, site2group, group2site, group2size, occurrences, top);
}

BaseMolecule* MoleculeRGroupsComposition::decorate(BaseMolecule &mol, Attachment &at) {
    return &mol.asMolecule();
    //todo
}

// temporary {
FileOutput MoleculeRGroupsComposition::file("out");
StandardOutput MoleculeRGroupsComposition::std;
// } temporary