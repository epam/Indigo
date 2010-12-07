/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_io.h"
#include "indigo_reaction.h"
#include "reaction/reaction_auto_loader.h"
#include "reaction/rxnfile_saver.h"
#include "base_cpp/output.h"
#include "reaction/reaction_automapper.h"
#include "base_cpp/auto_ptr.h"

IndigoBaseReaction::IndigoBaseReaction (int type_) : IndigoObject(type_)
{
}

IndigoBaseReaction::~IndigoBaseReaction ()
{
}

ReactionHighlighting * IndigoBaseReaction::getReactionHighlighting ()
{
   return &highlighting;
}

RedBlackStringObjMap< Array<char> > * IndigoBaseReaction::getProperties ()
{
   return &properties;
}

IndigoReaction::IndigoReaction () : IndigoBaseReaction(REACTION)
{
}

IndigoReaction::~IndigoReaction ()
{
}

Reaction & IndigoReaction::getReaction ()
{
   return rxn;
}

BaseReaction & IndigoReaction::getBaseReaction ()
{
   return rxn;
}

const char * IndigoReaction::getName ()
{
   if (rxn.name.ptr() == 0)
      return "";
   return rxn.name.ptr();
}

IndigoQueryReaction::IndigoQueryReaction () : IndigoBaseReaction(QUERY_REACTION)
{
}

IndigoQueryReaction::~IndigoQueryReaction ()
{
}

BaseReaction & IndigoQueryReaction::getBaseReaction ()
{
   return rxn;
}

QueryReaction & IndigoQueryReaction::getQueryReaction ()
{
   return rxn;
}

const char * IndigoQueryReaction::getName ()
{
   if (rxn.name.ptr() == 0)
      return "";
   return rxn.name.ptr();
}

IndigoReactionMolecule::IndigoReactionMolecule (BaseReaction &reaction, ReactionHighlighting *highlighting, int index) :
IndigoObject(REACTION_MOLECULE),
rxn(reaction),
idx(index)
{
   hl = highlighting;
}

IndigoReactionMolecule::~IndigoReactionMolecule ()
{
}

BaseMolecule & IndigoReactionMolecule::getBaseMolecule ()
{
   return rxn.getBaseMolecule(idx);
}

Molecule & IndigoReactionMolecule::getMolecule ()
{
   return rxn.getBaseMolecule(idx).asMolecule();
}

QueryMolecule & IndigoReactionMolecule::getQueryMolecule ()
{
   return rxn.getBaseMolecule(idx).asQueryMolecule();
}

GraphHighlighting * IndigoReactionMolecule::getMoleculeHighlighting ()
{
   if (hl == 0)
      return 0;
   hl->nondestructiveInit(rxn);
   return &hl->getGraphHighlighting(idx);
}

int IndigoReactionMolecule::getIndex ()
{
   return idx;
}


IndigoReactionIter::IndigoReactionIter (BaseReaction &rxn, ReactionHighlighting *hl, int subtype) :
IndigoObject(REACTION_ITER),
_rxn(rxn)
{
   _subtype = subtype;
   _idx = -1;
   _hl = hl;
}

IndigoReactionIter::~IndigoReactionIter ()
{
}

int IndigoReactionIter::_begin ()
{
   if (_subtype == REACTANTS)
      return _rxn.reactantBegin();
   if (_subtype == PRODUCTS)
      return _rxn.productBegin();

   return _rxn.begin();
}

int IndigoReactionIter::_end ()
{
   if (_subtype == REACTANTS)
      return _rxn.reactantEnd();
   if (_subtype == PRODUCTS)
      return _rxn.productEnd();

   return _rxn.end();
}

int IndigoReactionIter::_next (int i)
{
   if (_subtype == REACTANTS)
      return _rxn.reactantNext(i);
   if (_subtype == PRODUCTS)
      return _rxn.productNext(i);

   return _rxn.next(i);
}

IndigoObject * IndigoReactionIter::next ()
{
   if (_idx == -1)
   {
      _idx = _begin();
   }
   else
      _idx = _next(_idx);

   if (_idx == _end())
      return 0;

   return new IndigoReactionMolecule(_rxn, _hl, _idx);
}

bool IndigoReactionIter::hasNext ()
{
   if (_idx == -1)
      return _begin() != _end();

   return _next(_idx) != _end();
}

IndigoObject * IndigoReaction::clone ()
{
   AutoPtr<IndigoReaction> rxnptr;
   rxnptr.reset(new IndigoReaction());
   rxnptr->rxn.clone(rxn, 0, 0);
   rxnptr->highlighting.init(rxnptr->rxn);
   rxnptr->copyProperties(properties);
   return rxnptr.release();
}

IndigoObject * IndigoQueryReaction::clone ()
{
   AutoPtr<IndigoQueryReaction> rxnptr;
   rxnptr.reset(new IndigoQueryReaction());
   rxnptr->rxn.clone(rxn, 0, 0);
   rxnptr->highlighting.init(rxnptr->rxn);
   rxnptr->copyProperties(properties);
   return rxnptr.release();
}

int _indigoIterateReaction (int reaction, int subtype)
{
   INDIGO_BEGIN
   {
      BaseReaction &rxn = self.getObject(reaction).getBaseReaction();
      ReactionHighlighting *hl = self.getObject(reaction).getReactionHighlighting();

      return self.addObject(new IndigoReactionIter(rxn, hl, subtype));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoLoadReaction (int source)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(source);
      Scanner &scanner = IndigoScanner::get(obj);

      ReactionAutoLoader loader(scanner);

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;

      AutoPtr<IndigoReaction> rxnptr(new IndigoReaction());

      Reaction &rxn = rxnptr->rxn;
      loader.highlighting = &rxnptr->highlighting;

      loader.loadReaction(rxn);
      if (rxnptr->highlighting.getCount() == 0)
         rxnptr->highlighting.init(rxnptr->rxn);
      return self.addObject(rxnptr.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoLoadQueryReaction (int source)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(source);
      Scanner &scanner = IndigoScanner::get(obj);

      ReactionAutoLoader loader(scanner);

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;

      AutoPtr<IndigoQueryReaction> rxnptr(new IndigoQueryReaction());

      QueryReaction &rxn = rxnptr->rxn;
      loader.highlighting = &rxnptr->highlighting;

      loader.loadQueryReaction(rxn);
      if (rxnptr->highlighting.getCount() == 0)
         rxnptr->highlighting.init(rxnptr->rxn);

      return self.addObject(rxnptr.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSaveRxnfile (int reaction, int output)
{
   INDIGO_BEGIN
   {
      BaseReaction &rxn = self.getObject(reaction).getBaseReaction();
      Output &out = IndigoOutput::get(self.getObject(output));
      
      RxnfileSaver saver(out);
      if (rxn.isQueryReaction())
         saver.saveQueryReaction(rxn.asQueryReaction());
      else
         saver.saveReaction(rxn.asReaction());
      out.flush();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateReactants (int reaction)
{
   return _indigoIterateReaction(reaction, IndigoReactionIter::REACTANTS);
}

CEXPORT int indigoIterateProducts (int reaction)
{
   return _indigoIterateReaction(reaction, IndigoReactionIter::PRODUCTS);
}

CEXPORT int indigoIterateMolecules (int reaction)
{
   return _indigoIterateReaction(reaction, IndigoReactionIter::MOLECULES);
}

CEXPORT int indigoCreateReaction (void)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoReaction());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCreateQueryReaction (void)
{
   return indigoCreateReaction();
}

CEXPORT int indigoAddReactant (int reaction, int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &robj = self.getObject(reaction);
      BaseReaction &rxn = robj.getBaseReaction();

      rxn.addReactantCopy(self.getObject(molecule).getBaseMolecule(), 0, 0);
      ReactionHighlighting *hl = robj.getReactionHighlighting();
      if (hl != 0)
         hl->nondestructiveInit(rxn);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAddProduct (int reaction, int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &robj = self.getObject(reaction);
      BaseReaction &rxn = robj.getBaseReaction();

      rxn.addProductCopy(self.getObject(molecule).getBaseMolecule(), 0, 0);
      ReactionHighlighting *hl = robj.getReactionHighlighting();
      if (hl != 0)
         hl->nondestructiveInit(rxn);
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountReactants (int reaction)
{
   INDIGO_BEGIN
   {
      return self.getObject(reaction).getBaseReaction().reactantsCount();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountProducts (int reaction)
{
   INDIGO_BEGIN
   {
      return self.getObject(reaction).getBaseReaction().productsCount();
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountMolecules (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseReaction())
         return obj.getBaseReaction().count();
      
      throw IndigoError("can not count molecules of %s", obj.debugInfo());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoAutomap (int reaction, const char *mode)
{
   INDIGO_BEGIN
   {
      BaseReaction &rxn = self.getObject(reaction).getBaseReaction();
      int nmode;

      if (mode == 0 || mode[0] == 0 || strcasecmp(mode, "discard") == 0)
         nmode = ReactionAutomapper::AAM_REGEN_DISCARD;
      else if (strcasecmp(mode, "alter") == 0)
         nmode = ReactionAutomapper::AAM_REGEN_ALTER;
      else if (strcasecmp(mode, "keep") == 0)
         nmode = ReactionAutomapper::AAM_REGEN_KEEP;
      else if (strcasecmp(mode, "clear") == 0)
      {
         rxn.clearAAM();
         return 0;
      }
      else
         throw IndigoError("indigoAutomap(): unknown mode: %s", mode);

      ReactionAutomapper ram(rxn);

      ram.automap(nmode);
      return 1;
   }
   INDIGO_END(-1);
}
