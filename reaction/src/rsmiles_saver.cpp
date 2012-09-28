/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "reaction/rsmiles_saver.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"

#include "molecule/smiles_saver.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"

using namespace indigo;

IMPL_ERROR(RSmilesSaver, "reaction SMILES saver");

RSmilesSaver::RSmilesSaver (Output &output) :
_output(output),
TL_CP_GET(_written_atoms),
TL_CP_GET(_written_bonds),
TL_CP_GET(_ncomp)
{
   smarts_mode = false;
}

void RSmilesSaver::saveReaction (Reaction &reaction)
{
   _rxn = &reaction;
   _brxn = &reaction;
   _qrxn = 0;
   _saveReaction();
}

void RSmilesSaver::saveQueryReaction (QueryReaction &reaction)
{
   _qrxn = &reaction;
   _brxn = &reaction;
   _rxn = 0;
   _saveReaction();
}

void RSmilesSaver::_writeMolecule (int i)
{
   SmilesSaver saver(_output);

   saver.write_extra_info = false;
   saver.separate_rsites = false;
   saver.rsite_indices_as_aam = false;
   saver.atom_atom_mapping = _brxn->getAAMArray(i).ptr();
   saver.smarts_mode = smarts_mode;

   if (_rxn != 0)
      saver.saveMolecule(_rxn->getMolecule(i));
   else
      saver.saveQueryMolecule(_qrxn->getQueryMolecule(i));

   _ncomp.push(saver.writtenComponents());

   const Array<int> &atoms = saver.writtenAtoms();
   int j;

   for (j = 0; j < atoms.size(); j++)
   {
      _Idx &idx = _written_atoms.push();

      idx.mol = i;
      idx.idx = atoms[j];
   }

   const Array<int> &bonds = saver.writtenBonds();

   for (j = 0; j < bonds.size(); j++)
   {
      _Idx &idx = _written_bonds.push();

      idx.mol = i;
      idx.idx = bonds[j];
   }
}

void RSmilesSaver::_saveReaction ()
{
   int i;

   _written_atoms.clear();
   _written_bonds.clear();
   _ncomp.clear();
   _comma = false;

   bool dot = false;

   for (i = _brxn->reactantBegin(); i != _brxn->reactantEnd(); i = _brxn->reactantNext(i))
   {
      if (dot)
         _output.writeChar('.');
      else
         dot = true;

      _writeMolecule(i);
   }

   _output.writeString(">");

   dot = false;
   for (i = _brxn->catalystBegin(); i != _brxn->catalystEnd(); i = _brxn->catalystNext(i))
   {
      if (dot)
         _output.writeChar('.');
      else
         dot = true;

      _writeMolecule(i);
   }

   _output.writeString(">");

   dot = false;
   for (i = _brxn->productBegin(); i != _brxn->productEnd(); i = _brxn->productNext(i))
   {
      if (dot)
         _output.writeChar('.');
      else
         dot = true;

      _writeMolecule(i);
   }

   _writeFragmentsInfo();
   _writeStereogroups();
   _writeRadicals();
   _writePseudoAtoms();
   _writeHighlighting();

   if (_comma)
      _output.writeChar('|');
}

void RSmilesSaver::_writeFragmentsInfo ()
{
   int i, j, cnt = 0;

   for (i = 0; i < _ncomp.size(); i++)
   {
      if (_ncomp[i] > 1)
         break;
      cnt += _ncomp[i];
   }

   if (i == _ncomp.size())
      return;

   if (_comma)
      _output.writeChar(',');
   else
   {
      _output.writeString(" |");
      _comma = true;
   }

   _output.writeString("f:");

   bool comma = false;

   for (; i < _ncomp.size(); i++)
   {
      if (_ncomp[i] > 1)
      {
         if (comma)
            _output.writeChar(',');
         else
            comma = true;
         _output.printf("%d", cnt);
         for (j = 1; j < _ncomp[i]; j++)
            _output.printf(".%d", cnt + j);
      }
      cnt += _ncomp[i];
   }
}

void RSmilesSaver::_writeStereogroups ()
{
   QS_DEF(Array<int>, marked);
   int i, j;

   for (i = 0; i < _written_atoms.size(); i++)
   {
      const _Idx &idx = _written_atoms[i];
      int type = _brxn->getBaseMolecule(idx.mol).stereocenters.getType(idx.idx);

      if (type != 0 && type != MoleculeStereocenters::ATOM_ABS)
         break;
   }

   if (i == _written_atoms.size())
      return;

   marked.clear_resize(_written_atoms.size());
   marked.zerofill();

   int and_group_idx = 1;
   int or_group_idx = 1;

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (marked[i])
         continue;

      const _Idx &idx = _written_atoms[i];

      int type = _brxn->getBaseMolecule(idx.mol).stereocenters.getType(idx.idx);

      if (type > 0)
      {
         if (_comma)
            _output.writeChar(',');
         else
         {
            _output.writeString(" |");
            _comma = true;
         }
      }

      if (type == MoleculeStereocenters::ATOM_ANY)
      {
         _output.printf("w:%d", i);

         for (j = i + 1; j < _written_atoms.size(); j++)
         {
            const _Idx &jdx = _written_atoms[j];

            if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_ANY)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
         }
      }
      else if (type == MoleculeStereocenters::ATOM_ABS)
      {
         _output.printf("a:%d", i);

         for (j = i + 1; j < _written_atoms.size(); j++)
         {
            const _Idx &jdx = _written_atoms[j];

            if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_ABS)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
         }
      }
      else if (type == MoleculeStereocenters::ATOM_AND)
      {
         int group = _brxn->getBaseMolecule(idx.mol).stereocenters.getGroup(idx.idx);

         _output.printf("&%d:%d", and_group_idx++, i);
         for (j = i + 1; j < _written_atoms.size(); j++)
         {
            const _Idx &jdx = _written_atoms[j];

            if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_AND &&
                _brxn->getBaseMolecule(jdx.mol).stereocenters.getGroup(jdx.idx) == group)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
         }
      }
      else if (type == MoleculeStereocenters::ATOM_OR)
      {
         int group = _brxn->getBaseMolecule(idx.mol).stereocenters.getGroup(idx.idx);

         _output.printf("&%d:%d", or_group_idx++, i);
         for (j = i + 1; j < _written_atoms.size(); j++)
         {
            const _Idx &jdx = _written_atoms[j];

            if (_brxn->getBaseMolecule(jdx.mol).stereocenters.getType(jdx.idx) == MoleculeStereocenters::ATOM_OR &&
                _brxn->getBaseMolecule(jdx.mol).stereocenters.getGroup(jdx.idx) == group)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
         }
      }
   }
}

void RSmilesSaver::_writeRadicals ()
{
   QS_DEF(Array<int>, marked);
   int i, j;

   marked.clear_resize(_written_atoms.size());
   marked.zerofill();

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (marked[i])
         continue;

      const _Idx &idx = _written_atoms[i];
      BaseMolecule &bmol = _brxn->getBaseMolecule(idx.mol);

      if (bmol.isRSite(idx.idx) || bmol.isPseudoAtom(idx.idx))
         continue;

      int radical = bmol.getAtomRadical(idx.idx);

      if (radical <= 0)
         continue;

      if (_comma)
         _output.writeChar(',');
      else
      {
         _output.writeString(" |");
         _comma = true;
      }

      if (radical == RADICAL_SINGLET)
         _output.writeString("^3:");
      else if (radical == RADICAL_DOUBLET)
         _output.writeString("^1:");
      else // RADICAL_TRIPLET
         _output.writeString("^4:");

      _output.printf("%d", i);

      for (j = i + 1; j < _written_atoms.size(); j++)
      {
         const _Idx &jdx = _written_atoms[j];

         if (_brxn->getBaseMolecule(jdx.mol).getAtomRadical(jdx.idx) == radical)
         {
            marked[j] = 1;
            _output.printf(",%d", j);
         }
      }
   }
}

void RSmilesSaver::_writePseudoAtoms ()
{
   int i;

   for (i = 0; i < _written_atoms.size(); i++)
   {
      BaseMolecule &mol = _brxn->getBaseMolecule(_written_atoms[i].mol);
      int idx = _written_atoms[i].idx;

      if (mol.isPseudoAtom(idx))
         break;
      if (mol.isRSite(idx) && mol.getRSiteBits(idx) > 0)
         break;
   }

   if (i == _written_atoms.size())
      return;
   
   if (_comma)
      _output.writeChar(',');
   else
   {
      _output.writeString(" |");
      _comma = true;
   }

   _output.writeChar('$');

   for (i = 0; i < _written_atoms.size(); i++)
   {
      _Idx &idx = _written_atoms[i];

      if (i > 0)
         _output.writeChar(';');

      BaseMolecule &mol = _brxn->getBaseMolecule(idx.mol);

      if (mol.isPseudoAtom(idx.idx))
         SmilesSaver::writePseudoAtom(mol.getPseudoAtom(idx.idx), _output);
      else if (mol.isRSite(idx.idx) && mol.getRSiteBits(idx.idx) > 0)
         // ChemAxon's Extended SMILES notation for R-sites
         _output.printf("_R%d", mol.getSingleAllowedRGroup(idx.idx));

   }

   _output.writeChar('$');
}

void RSmilesSaver::_writeHighlighting ()
{
   int i;

   bool ha = false;

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (_brxn->getBaseMolecule(_written_atoms[i].mol).isAtomHighlighted(_written_atoms[i].idx))
      {
         if (ha)
            _output.writeChar(',');
         else
         {
            if (_comma)
               _output.writeChar(',');
            else
            {
               _output.writeString(" |");
               _comma = true;
            }
            _output.writeString("ha:");
            ha = true;
         }

         _output.printf("%d", i);
      }
   }

   bool hb = false;

   for (i = 0; i < _written_bonds.size(); i++)
   {
      if (_brxn->getBaseMolecule(_written_bonds[i].mol).isBondHighlighted(_written_bonds[i].idx))
      {
         if (hb)
            _output.writeChar(',');
         else
         {
            if (_comma)
               _output.writeChar(',');
            else
            {
               _output.writeString(" |");
               _comma = true;
            }
            _output.writeString("hb:");
            hb = true;
         }

         _output.printf("%d", i);
      }
   }
}
