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

#ifndef __rxnfile_loader__
#define __rxnfile_loader__

#include "base_cpp/exception.h"

namespace indigo {

class Scanner;
class BaseReaction;
class Reaction;
class QueryReaction;
class MolfileLoader;

class DLLEXPORT RxnfileLoader {
public:
   RxnfileLoader (Scanner &scanner);
   ~RxnfileLoader ();

   void loadReaction (Reaction& reaction);
   void loadQueryReaction (QueryReaction& reaction);


   bool treat_x_as_pseudoatom;
   bool ignore_stereocenter_errors;
   bool ignore_noncritical_query_features;
   
   DECL_ERROR;
protected:

   BaseReaction  *_brxn;
   QueryReaction *_qrxn;
   Reaction      *_rxn;
   
   void _loadReaction ();

   Scanner &_scanner;
   void _readRxnHeader();
   void _readReactantsHeader();
   void _readProductsHeader();
   void _readCatalystsHeader();
   void _readReactantsFooter();
   void _readProductsFooter();
   void _readCatalystsFooter();
   void _readMolHeader();
   void _readMol (MolfileLoader &loader, int index);
   int _n_reactants;
   int _n_products;
   int _n_catalysts;
   bool _v3000;
};

}

#endif
