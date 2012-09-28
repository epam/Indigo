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


#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/rxnfile_loader.h"
#include "molecule/molfile_loader.h"
#include "base_cpp/scanner.h"

using namespace indigo;

IMPL_ERROR(RxnfileLoader, "RXN loader");

RxnfileLoader::RxnfileLoader (Scanner& scanner): _scanner(scanner){
   _v3000 = false;
   ignore_stereocenter_errors = false;
   ignore_noncritical_query_features = false;
}

RxnfileLoader::~RxnfileLoader(){
}

void RxnfileLoader::loadReaction(Reaction &reaction){
   _rxn = &reaction;
   _brxn = &reaction;
   _qrxn = 0;
   _loadReaction();
}

void RxnfileLoader::loadQueryReaction(QueryReaction& rxn){
   _rxn = 0;
   _brxn = &rxn;
   _qrxn = &rxn;
   _loadReaction();
}


void RxnfileLoader::_loadReaction(){
   _brxn->clear();

   MolfileLoader molfileLoader(_scanner);

   molfileLoader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
   molfileLoader.ignore_stereocenter_errors = ignore_stereocenter_errors;
   molfileLoader.ignore_noncritical_query_features = ignore_noncritical_query_features;
   _readRxnHeader();

   _readReactantsHeader();
   for (int i = 0; i < _n_reactants; i++) {
      int index = _brxn->addReactant();

      _readMolHeader();
      _readMol(molfileLoader, index);
   }
   _readReactantsFooter();

   _readProductsHeader();
   for (int i = 0; i < _n_products; i++) {
      int index = _brxn->addProduct();

      _readMolHeader();
      _readMol(molfileLoader, index);
   }
   _readProductsFooter();

   if (_n_catalysts > 0)
   {
      _readCatalystsHeader();
      for (int i = 0; i < _n_catalysts; i++) {
         int index = _brxn->addCatalyst();

         _readMolHeader();
         _readMol(molfileLoader, index);
      }
      _readCatalystsFooter();
   }
}

void RxnfileLoader::_readRxnHeader(){

   QS_DEF(Array<char>, header);

   _scanner.readLine(header, true);

   if (strcmp(header.ptr(), "$RXN") == 0)
      _v3000 = false;
   else if (strcmp(header.ptr(), "$RXN V3000") == 0)
      _v3000 = true;
   else
      throw Error("bad header %s", header.ptr());

   _scanner.readLine(_brxn->name, true);
   _scanner.skipLine();
   _scanner.skipLine();

   if (_v3000)
   {
      _scanner.skip(14); // "M  V30 COUNTS "
      _scanner.readLine(header, true);
      int n = sscanf(header.ptr(), "%d %d %d", &_n_reactants, &_n_products, &_n_catalysts);
      if (n < 2)
         throw Error("error reading counts: %s", header.ptr());
      if (n == 2)
         _n_catalysts = 0;
   }
   else
   {
      _scanner.readLine(header, false);
      BufferScanner strscan(header);

      _n_reactants = strscan.readIntFix(3);
      _n_products = strscan.readIntFix(3);
      try
      {
         _n_catalysts = strscan.readIntFix(3);
      }
      catch (Scanner::Error)
      {
         _n_catalysts = 0;
      }
   }
}

void RxnfileLoader::_readProductsHeader(){
   if (!_v3000) {
      return;
   }

   QS_DEF(Array<char>, header);

   _scanner.readLine(header, true);
   if (strcmp(header.ptr(), "M  V30 BEGIN PRODUCT") != 0){
      throw Error("bad products header: %s", header.ptr());
   }
}

void RxnfileLoader::_readReactantsHeader(){
   if (!_v3000) {
      return;
   }

   QS_DEF(Array<char>, header);

   _scanner.readLine(header, true);
   if (strcmp(header.ptr(), "M  V30 BEGIN REACTANT") != 0){
      throw Error("bad reactants header: %s", header.ptr());
   }
}

void RxnfileLoader::_readCatalystsHeader(){
   if (!_v3000) {
      return;
   }

   QS_DEF(Array<char>, header);

   _scanner.readLine(header, true);
   if (strcmp(header.ptr(), "M  V30 BEGIN AGENT") != 0){
      throw Error("bad catalysts header: %s", header.ptr());
   }
}

void RxnfileLoader::_readMolHeader(){

   if (_v3000) {
      return;
   }
   QS_DEF(Array<char>, header);

   _scanner.readLine(header, true);
   if (strcmp(header.ptr(), "$MOL") != 0)
      throw Error("bad molecule header: %s", header.ptr());
}

void RxnfileLoader::_readReactantsFooter(){
   if (!_v3000) {
      return;
   }
   QS_DEF(Array<char>, footer);

   _scanner.readLine(footer, true);

   if (strcmp(footer.ptr(), "M  V30 END REACTANT") != 0)
      throw Error("bad reactants footer: %s", footer.ptr());
}

void RxnfileLoader::_readProductsFooter(){
   if (!_v3000) {
      return;
   }
   QS_DEF(Array<char>, footer);

   _scanner.readLine(footer, true);

   if (strcmp(footer.ptr(), "M  V30 END PRODUCT") != 0)
      throw Error("bad products footer: %s", footer.ptr());
}

void RxnfileLoader::_readCatalystsFooter(){
   if (!_v3000) {
      return;
   }
   QS_DEF(Array<char>, footer);

   _scanner.readLine(footer, true);

   if (strcmp(footer.ptr(), "M  V30 END AGENT") != 0)
      throw Error("bad agents footer: %s", footer.ptr());
}


void RxnfileLoader::_readMol (MolfileLoader &loader, int index) {

   loader.reaction_atom_mapping = &_brxn->getAAMArray(index);
   loader.reaction_atom_inversion = &_brxn->getInversionArray(index);
   loader.reaction_bond_reacting_center = &_brxn->getReactingCenterArray(index);

   if (_qrxn != 0)
      loader.reaction_atom_exact_change = &_qrxn->getExactChangeArray(index);

   if (_qrxn != 0)
   {
      if (_v3000)
         loader.loadQueryCtab3000(_qrxn->getQueryMolecule(index));
      else
         loader.loadQueryMolecule(_qrxn->getQueryMolecule(index));
   }
   else
   {
      if (_v3000)
         loader.loadCtab3000(_rxn->getMolecule(index));
      else
         loader.loadMolecule(_rxn->getMolecule(index));
   }
}
