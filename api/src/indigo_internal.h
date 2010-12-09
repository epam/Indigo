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

#ifndef __indigo_internal__
#define __indigo_internal__

#include "indigo.h"

#include "base_cpp/exception.h"
#include "base_cpp/io_base.h"

#include "option_manager.h"
#include "molecule/molecule_fingerprint.h"

using namespace indigo;

namespace indigo
{
   class GraphHighlighting;
   class BaseReaction;
   class QueryReaction;
   class Reaction;
   class ReactionHighlighting;
class Output;
class Scanner;
class SdfLoader;
class RdfLoader;
}

extern DLLEXPORT OptionManager & indigoGetOptionManager ();

class IndigoObject
{
public:
   explicit DLLEXPORT IndigoObject (int type_);
   virtual DLLEXPORT ~IndigoObject ();

   enum
   {
      SCANNER = 1,
      MOLECULE,
      QUERY_MOLECULE,
      REACTION,
      QUERY_REACTION,
      OUTPUT,
      REACTION_ITER,
      REACTION_MOLECULE,
      GROSS,
      SDF_LOADER,
      SDF_SAVER,
      RDF_MOLECULE,
      RDF_REACTION,
      RDF_LOADER,
      SMILES_MOLECULE,
      SMILES_REACTION,
      MULTILINE_SMILES_LOADER,
      ATOM,
      ATOMS_ITER,
      RGROUP,
      RGROUPS_ITER,
      RGROUP_FRAGMENT,
      RGROUP_FRAGMENTS_ITER,
      ARRAY,
      ARRAY_ITER,
      ARRAY_ELEMENT,
      MOLECULE_SUBSTRUCTURE_MATCH,
      MOLECULE_SUBSTRUCTURE_MATCH_ITER,
      MOLECULE_SUBSTRUCTURE_MATCHER,
      SCAFFOLD,
      DECONVOLUTION,
      DECONVOLUTION_ELEM,
      DECONVOLUTION_ITER,
      PROPERTIES_ITER,
      PROPERTY,
      FINGERPRINT,
      BOND,
      BONDS_ITER,
      ATOM_NEIGHBOR,
      ATOM_NEIGHBORS_ITER
   };

   int type;

   virtual DLLEXPORT const char * debugInfo ();

   virtual DLLEXPORT void toString (Array<char> &str);
   virtual DLLEXPORT void toBuffer (Array<char> &buf);
   virtual DLLEXPORT BaseMolecule & getBaseMolecule ();
   virtual DLLEXPORT QueryMolecule & getQueryMolecule ();
   virtual DLLEXPORT Molecule & getMolecule ();
   virtual DLLEXPORT GraphHighlighting * getMoleculeHighlighting();

   virtual DLLEXPORT BaseReaction & getBaseReaction ();
   virtual DLLEXPORT QueryReaction & getQueryReaction ();
   virtual DLLEXPORT Reaction & getReaction ();
   virtual DLLEXPORT ReactionHighlighting * getReactionHighlighting();

   virtual DLLEXPORT RedBlackStringObjMap< Array<char> > * getProperties();
   
   virtual DLLEXPORT IndigoObject * clone ();

   virtual DLLEXPORT const char * getName ();

   virtual DLLEXPORT int getIndex ();

   virtual DLLEXPORT IndigoObject * next ();
   
   virtual DLLEXPORT bool hasNext ();

   bool DLLEXPORT isBaseMolecule ();
   bool DLLEXPORT isBaseReaction ();

   void DLLEXPORT copyProperties (RedBlackStringObjMap< Array<char> > &other);

protected:
   Array<char> _dbg_info; // allocated by debugInfo() on demand
};

class IndigoGross : public IndigoObject
{
public:
   IndigoGross ();
   virtual ~IndigoGross ();

   virtual void toString (Array<char> &str);

   Array<int> gross;
};

struct ProductEnumeratorParams
{
   ProductEnumeratorParams ()
   {
      clear();
   }

   void clear ()
   {
      is_multistep_reactions = false;
      is_one_tube = false;
      is_self_react = false;
      max_deep_level = 2;
      max_product_count = 1000;
   }

   bool is_multistep_reactions;
   bool is_one_tube;
   bool is_self_react;
   int max_deep_level;
   int max_product_count;
};

class Indigo
{
public:
   Indigo ();
   ~Indigo ();

   Array<char> error_message;
   INDIGO_ERROR_HANDLER error_handler;
   void  *error_handler_context;

   DLLEXPORT IndigoObject & getObject (int handle);
   DLLEXPORT int countObjects ();

   DLLEXPORT int addObject (IndigoObject *obj);

   DLLEXPORT void removeObject (int id);

   DLLEXPORT void removeAllObjects ();

   Array<char> tmp_string;
   float tmp_xyz[3];

   ProductEnumeratorParams rpe_params;

   MoleculeFingerprintParameters fp_params;
   
   bool ignore_stereochemistry_errors;
   bool treat_x_as_pseudoatom;

   bool deconvolution_aromatization;

   int  molfile_saving_mode; // MolfileSaver::MODE_***, default is zero

   Encoding filename_encoding;

   bool embedding_edges_uniqueness;
   int max_embeddings;

protected:

   RedBlackMap<int, IndigoObject *> _objects;

   int    _next_id;
   OsLock _objects_lock;
};



#define INDIGO_BEGIN { Indigo &self = indigoGetInstance(); \
      try { self.error_message.clear();

#define INDIGO_END(fail) } \
      catch (Exception &ex)         \
      {                             \
          self.error_message.readString(ex.message(), true); \
          if (self.error_handler != 0)                       \
             self.error_handler(ex.message(),                \
                                self.error_handler_context); \
          return fail;                                       \
      } }


#define INDIGO_END_CHECKMSG(success, fail) } \
      catch (Exception &ex)         \
      {                             \
         self.error_message.readString(ex.message(), true); \
         if (self.error_handler != 0)                       \
            self.error_handler(ex.message(),                \
                               self.error_handler_context); \
         return fail;                                       \
      }                                                     \
      if (self.error_message.size() > 0)                    \
      {                                                     \
         if (self.error_handler != 0)                       \
            self.error_handler(self.error_message.ptr(),    \
                               self.error_handler_context); \
         return fail;                                       \
      }                                                     \
      return success; }

DLLEXPORT Indigo & indigoGetInstance ();

class IndigoError : public Exception
{
public:
   explicit DLLEXPORT IndigoError (const char *format, ...);
   DLLEXPORT IndigoError (const IndigoError &);
private:
};

class _IndigoBasicOptionsHandlersSetter
{
public:
   DLLEXPORT _IndigoBasicOptionsHandlersSetter ();
   DLLEXPORT ~_IndigoBasicOptionsHandlersSetter ();
};

#endif
