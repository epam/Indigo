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

#include "base_cpp/scanner.h"
#include "graph/embedding_enumerator.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_tautomer_matcher.h"
#include "molecule/molecule_tautomer_utils.h"
#include "molecule/elements.h"

using namespace indigo;

IMPL_ERROR(MoleculeTautomerMatcher, "molecule tautomer matcher");

MoleculeTautomerMatcher::MoleculeTautomerMatcher (Molecule &target, bool substructure) :
_substructure(substructure),
_force_hydrogens(false),
_ring_chain(false),
_rules(0),
_rules_list(0),
_context(0),
_target_src(target),
_query(0)
{
   if (substructure)
   {
      _target.create(target);
      _supermol = _target.get();
   }
   else
      _supermol = &target;

   _target_decomposer.create(*_supermol);
   _target_decomposer->decompose();

   highlight = false;
}

void MoleculeTautomerMatcher::setRulesList (const PtrArray<TautomerRule> *rules_list)
{
   _rules_list = rules_list;
}

void MoleculeTautomerMatcher::setRules (int rules_set, bool force_hydrogens, bool ring_chain)
{
   _rules = rules_set;
   _force_hydrogens = force_hydrogens;
   _ring_chain = ring_chain;
}

void MoleculeTautomerMatcher::setQuery (BaseMolecule &query)
{
   if (_substructure)
      _query.reset(new QueryMolecule());
   else
      _query.reset(new Molecule());
   
   _query->clone(query, 0, 0, 0);
   _query_decomposer.create(query);
   _query_decomposer->decompose();
}

int MoleculeTautomerMatcher::countNonHydrogens (BaseMolecule &molecule)
{
   int count = 0;

   for (int i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
      if (molecule.getAtomNumber(i) != ELEM_H)
         count++;

   return count;
}

bool MoleculeTautomerMatcher::find ()
{
   if (!_substructure)
   {
      int nh_count1 = countNonHydrogens(*_query.get());
      int nh_count2 = countNonHydrogens(*_supermol);

      if (nh_count1 != nh_count2)
         return false;
   }

   const PtrArray<TautomerRule> rules_list;
   const PtrArray<TautomerRule> *p_rules_list = _rules_list;

   if (p_rules_list == 0)
      p_rules_list = &rules_list;

   _context.reset(new TautomerSearchContext(*_query.get(), *_supermol, *_query_decomposer.get(), 
      *_target_decomposer.get(), *p_rules_list, arom_options));

   _context->force_hydrogens = _force_hydrogens;
   _context->ring_chain = _ring_chain;
   _context->rules = _rules;

   if (_rules != 0 && _rules_list != 0 && _rules_list->size() != 0)
      _context->cb_check_rules = _checkRules;

   _context->substructure = _substructure;

   TautomerMatcher matcher(*_context.get());

   bool found = true;

   if (matcher.findMatch())
      found = false;

   if (found && highlight)
   {
      _target_src.unhighlightAll();

      if (_substructure)
         MoleculeTautomerUtils::highlightChains(*_query.get(), *_supermol, _context->chains_2, _context->core_2.ptr());
      else
         MoleculeTautomerUtils::highlightChains(*_query.get(), *_supermol, _context->chains_2, 0);

      if (_substructure)
         _target_src.highlightSubmolecule(*_supermol, _target->getInvMapping().ptr(), true);
   }

   return found;
}

const int * MoleculeTautomerMatcher::getQueryMapping ()
{
   return _context->core_1.ptr();
}

void MoleculeTautomerMatcher::parseConditions (const char *tautomer_text, int &rules, bool &force_hydrogens, bool &ring_chain)
{
   if (tautomer_text == 0)
      throw Error("zero pointer passed to parseConditions()");

   rules = 0;
   force_hydrogens = false;
   ring_chain = false;

   BufferScanner scanner(tautomer_text);

   QS_DEF(Array<char>, word);

   while (1)
   {
      int i;

      scanner.skipSpace();

      if (scanner.isEOF())
         break;

      scanner.readWord(word, 0);

      if (word.size() < 2)
         throw Error("internal error on token reading");

      if (strcasecmp(word.ptr(), "TAU") == 0)
         continue;
      if (strcasecmp(word.ptr(), "HYD") == 0)
      {
         force_hydrogens = true;
         continue;
      }
      if (strcasecmp(word.ptr(), "R-C") == 0)
      {
         ring_chain = true;
         continue;
      }
      if (strcasecmp(word.ptr(), "R*") == 0)
      {
         rules = 0xFFFFFF;
         continue;
      }
      if (word[0] == 'R' || word[0] == 'r')
      {
         if (isdigit(word[1]))
         {
            i = atoi(word.ptr() + 1);

            if (i > 0 && i <= 32)
            {
               rules |= 1 << (i - 1);
               continue;
            }
         }
      }
      throw Error("parseConditions(): unknown token %s", word.ptr());
   }
}


bool MoleculeTautomerMatcher::_checkRules(TautomerSearchContext &context, int first1, int first2, int last1, int last2)
{
   for (int i = 0; i < context.rules_list.size(); i++)
      if (context.rules & (1 << i) && context.rules_list[i] != 0)
      {
         if (context.rules_list[i]->check(context.g1, first1, last1,
            TautomerRule::atomInAromaticRing(context.g2, first2),
            TautomerRule::atomInAromaticRing(context.g2, last2)) &&
            context.rules_list[i]->check(context.g2, first2, last2,
            TautomerRule::atomInAromaticRing(context.g1, first1),
            TautomerRule::atomInAromaticRing(context.g1, last1)))
            return true;
      }

   return false;
}
