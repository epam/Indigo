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

Iterator<Attachment*>* MoleculeRGroupsComposition::refine() {
    MoleculeRGroups &rgroups = _mol.rgroups;
    auto k = rgroups.getRGroupCount();
    auto n = _mol.countRSites();

    MultiMap<int,int> site2label;
    MultiMap<int,int> label2site;
    for (auto i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i)) {
        if (!_mol.isRSite(i)) { continue; }

        int site = i + 1;
        Array<int> labels;
        _mol.getAllowedRGroups(i, labels);
        site2label.insert(site, labels);
        for (auto j = 0; j < labels.size(); j++) {
            label2site.insert(labels[j], site);
        }
    }

    { Array<char> out;
      out.appendString("site2label=", false);
      print(site2label, out, false);
      out.appendString("\nlabel2site=", false);
      print(label2site, out, true);
      printf("%s\n", out.ptr());
    }
    assert(site2label.size() == n);
    assert(label2site.size() == k);

    MultiMap<int,int> site2group;
    MultiMap<int,int> group2site;

    site2label.copy(site2group);
    label2site.copy(group2site);

//    for () {
        
//    }

//    bool withH = false;
    Topology top(k);
    RedBlackMap<int,int> group2size;
    OccurrenceRestrictions occurrences(k);
    for (auto label = 1; label <= k; label++) {
        const RGroup &rg = rgroups.getRGroup(label);
        if (rg.rest_h) {
            const RedBlackSet<int> &sites = label2site[label];
            for (auto i = sites.begin(); i != sites.end(); i += sites.next(i)) {
                auto site = sites.key(i);
                site2group.insert(site, label);
                site2group.insert(site, H);
                group2site.insert(H, site);
            }

            assert(false); //TODO
//            if (!withH) {
//                top.allow(H);
//                withH = true;
//            }

//            const Set &sites = group2site[label];
//            for (auto i = sites.begin(); i != sites.end(); i += sites.next(i)) {
//                auto site = sites.key(i);
//                site2group.remove(site);
//                site2group.insert(site, label);
//                site2group.insert(site, H);
//                group2site.insert(H, site);
//            }
        } else {
            //TODO
        }

        group2size.insert(label, rg.fragments.size());
        if (rg.if_then) {
            top.depends(label, rg.if_then);
        }
        occurrences.set(label, convert(rg.occurrence));
    }
    occurrences.set(H, natural());
    group2size.insert(H, 1);

    return (new Attachments(n, k, site2group, group2site, group2size, occurrences, top))->iterator();
}

BaseMolecule* MoleculeRGroupsComposition::decorate(Attachment &at) {
    Molecule* result = new Molecule();

    Molecule& mol = *result;
    mol.clone(_mol, nullptr, nullptr);

    MoleculeRGroups &rgroups = mol.rgroups;
    for (auto i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i)) {
        if (!mol.isRSite(i)) { continue; }
        int rsite = i;

        Fragment frag = at.at(rsite+1);
        BaseMolecule *fragment = rgroups.getRGroup(frag.group).fragments.at(frag.fragment-1);

        int apcount = fragment->attachmentPointCount();
        int apoint  = fragment->getAttachmentPoint(apcount, 0);

        Array<int> map;
        mol.mergeWithMolecule(*fragment, &map);

        int atom = mol.getAtomNumber(map[apoint]);
        if (mol.mergeAtoms(rsite, map[apoint]) == rsite) {
            mol.resetAtom(rsite, atom);
        }
    }

    rgroups.clear();
    mol.removeAttachmentPoints();

    return result;
}

Iterator<BaseMolecule*>* MoleculeRGroupsComposition::combinations() {
    std::function<BaseMolecule*(Attachment*)> at2mol = [this](Attachment *at) {
        return decorate(*at);
    };
    return map(at2mol, refine());
}