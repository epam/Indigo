/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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
#include "base_cpp/obj_array.h"
#include "graph/filter.h"
#include "layout/molecule_layout.h"

using namespace indigo;

MoleculeLayout::MoleculeLayout (BaseMolecule &molecule) :
_molecule(molecule)
{
   _hasMulGroups = _molecule.multiple_groups.size() > 0;
   _init();
   _query = _molecule.isQueryMolecule();
}

void MoleculeLayout::_init ()
{
   bond_length = 1.f;
   respect_existing_layout = false;
   filter = 0;
   max_iterations = 20;
   _query = false;
   _atomMapping.clear();

   _bm = &_molecule;
   if (_hasMulGroups) {
      if (_molecule.isQueryMolecule())
         _molCollapsed.reset(new QueryMolecule());
      else
         _molCollapsed.reset(new Molecule());
      _molCollapsed->clone(_molecule, &_atomMapping, NULL);
      QS_DEF(BaseMolecule::Mapping, atomMapCollapse);
      QS_DEF(BaseMolecule::Mapping, bondMapInv);
      for (int i = _molCollapsed->multiple_groups.begin(); i < _molCollapsed->multiple_groups.end(); i = _molCollapsed->multiple_groups.next(i)) {
         // collapse multiple group
         atomMapCollapse.clear();
         bondMapInv.clear();
         BaseMolecule::MultipleGroup::collapse(_molCollapsed.ref(), i, atomMapCollapse, bondMapInv);

         // modify the atom mapping
         for (int j = 0; j < _atomMapping.size(); ++j)
            if (atomMapCollapse.find(_atomMapping[j]))
               _atomMapping[j] = atomMapCollapse.at(_atomMapping[j]);
      }
      _bm = _molCollapsed.get();
   }

   _layout_graph.makeOnGraph(*_bm);

   for (int i = _layout_graph.vertexBegin(); i < _layout_graph.vertexEnd(); i = _layout_graph.vertexNext(i))
   {
      const Vec3f &pos = _bm->getAtomXyz(_layout_graph.getVertexExtIdx(i));

      _layout_graph.getPos(i).set(pos.x, pos.y);
   }
}

void MoleculeLayout::_make ()
{
   _layout_graph.max_iterations = max_iterations;
   if (filter != 0)
   {
      QS_DEF(Array<int>, fixed_vertices);

      fixed_vertices.clear_resize(_layout_graph.vertexEnd());
      fixed_vertices.zerofill();

      for (int i = _layout_graph.vertexBegin(); i < _layout_graph.vertexEnd(); i = _layout_graph.vertexNext(i))
         if (!filter->valid(_layout_graph.getVertexExtIdx(i)))
            fixed_vertices[i] = 1;

      Filter new_filter(fixed_vertices.ptr(), Filter::NEQ, 1);

      _layout_graph.layout(*_bm, bond_length, &new_filter, respect_existing_layout);
   } else
      _layout_graph.layout(*_bm, bond_length, 0, respect_existing_layout);


   for (int i = _layout_graph.vertexBegin(); i < _layout_graph.vertexEnd(); i = _layout_graph.vertexNext(i))
   {
      const LayoutVertex &vert = _layout_graph.getLayoutVertex(i);
      _bm->setAtomXyz(vert.ext_idx, vert.pos.x, vert.pos.y, 0.f);
   }

   if (_hasMulGroups) {
      for (int j = 0; j < _atomMapping.size(); ++j) {
         int i = _atomMapping[j];
         _molecule.setAtomXyz(j, _molCollapsed.ref().getAtomXyz(i));
      }
      _molCollapsed.reset(NULL);
   }

   _molecule.have_xyz = true;
}

Metalayout::LayoutItem& MoleculeLayout::_pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol)
{
   Metalayout::LayoutItem& item = line.items.push();
   item.type = 0;
   item.fragment = true;
   item.id = _map.size();
   _map.push(&mol);
   Metalayout::getBoundRect(item.min, item.max, mol);
   item.scaledSize.diff(item.max, item.min);
   return item;
}

BaseMolecule& MoleculeLayout::_getMol (int id)
{
   return *_map[id];
}
void MoleculeLayout::make ()
{
   _make();
   
   if (_query && _molecule.asQueryMolecule().rgroups.getRGroupCount() > 0)
   {
      MoleculeRGroups &rgs = ((QueryMolecule &)_molecule).rgroups;
      _ml.clear();
      _map.clear();
      _pushMol(_ml.newLine(), _molecule);
      for (int i = 0; i < rgs.getRGroupCount(); ++i)
      {
         RGroup& rg = rgs.getRGroup(i);
         Metalayout::LayoutLine& line = _ml.newLine();
         for (int j = 0; j < rg.fragments.size(); ++j)
         {
            QueryMolecule& mol = *rg.fragments[j];
            MoleculeLayout layout(mol);
            layout.max_iterations = max_iterations;
            layout.make();
            _pushMol(line, mol); // add molecule to metalayout AFTER its own layout is determined
         }
      }

      _ml.bondLength = bond_length;
      _ml.context = this;
      _ml.cb_getMol = cb_getMol;
      _ml.cb_process = cb_process;
      _ml.prepare();
      _ml.scaleSz();
      _ml.calcContentSize();
      _ml.process();
   }
}

BaseMolecule& MoleculeLayout::cb_getMol (int id, void* context)
{
   return ((MoleculeLayout*)context)->_getMol(id);
}  

void MoleculeLayout::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   MoleculeLayout* layout = (MoleculeLayout*)context;
   layout->_ml.adjustMol(layout->_getMol(item.id), item.min, pos);   
}

