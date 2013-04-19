/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#include "indigo.h"

#include "base_cpp/exception.h"
#include "base_cpp/io_base.h"
#include "base_cpp/cancellation_handler.h"

#include "option_manager.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_tautomer.h"

/* When Indigo internal code is used dynamically the INDIGO_VERSION define 
 * should be compared with indigoVersion() to ensure libraries binary 
 * compatibility. */
#include "indigo_version.h"

using namespace indigo;

namespace indigo
{
   class BaseReaction;
   class QueryReaction;
   class Reaction;
   class Output;
   class Scanner;
   class SdfLoader;
   class RdfLoader;
   class MolfileSaver;
   class RxnfileSaver;
}

extern DLLEXPORT OptionManager & indigoGetOptionManager ();

class DLLEXPORT IndigoObject
{
public:
   explicit IndigoObject (int type_);
   virtual ~IndigoObject ();

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
      MOLECULE_SUBSTRUCTURE_MATCH_ITER,
      MOLECULE_SUBSTRUCTURE_MATCHER,
      REACTION_SUBSTRUCTURE_MATCHER,
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
      ATOM_NEIGHBORS_ITER,
      SUPERATOM,
      SUPERATOMS_ITER,
      DATA_SGROUP,
      DATA_SGROUPS_ITER,
      REPEATING_UNIT,
      REPEATING_UNITS_ITER,
      MULTIPLE_GROUP,
      MULTIPLE_GROUPS_ITER,
      GENERIC_SGROUP,
      GENERIC_SGROUPS_ITER,
      SGROUP_ATOMS_ITER,
      SGROUP_BONDS_ITER,
      DECOMPOSITION,
      COMPONENT,
      COMPONENTS_ITER,
      COMPONENT_ATOMS_ITER,
      COMPONENT_BONDS_ITER,
      SUBMOLECULE,
      SUBMOLECULE_ATOMS_ITER,
      SUBMOLECULE_BONDS_ITER,
      MAPPING,
      REACTION_MAPPING,
      SSSR_ITER,
      SUBTREES_ITER,
      RINGS_ITER,
      EDGE_SUBMOLECULE_ITER,
      CML_MOLECULE,
      CML_REACTION,
      MULTIPLE_CML_LOADER,
      SAVER,
      ATTACHMENT_POINTS_ITER,
      DECOMPOSITION_MATCH,
      DECOMPOSITION_MATCH_ITER
   };

   int type;

   virtual const char * debugInfo ();

   virtual void toString (Array<char> &str);
   virtual void toBuffer (Array<char> &buf);
   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual Molecule & getMolecule ();

   virtual BaseReaction & getBaseReaction ();
   virtual QueryReaction & getQueryReaction ();
   virtual Reaction & getReaction ();

   virtual RedBlackStringObjMap< Array<char> > * getProperties();
   
   virtual IndigoObject * clone ();

   virtual const char * getName ();

   virtual int getIndex ();

   virtual IndigoObject * next ();
   
   virtual bool hasNext ();

   virtual void remove ();

   void copyProperties (RedBlackStringObjMap< Array<char> > &other);

protected:
   AutoPtr< Array<char> > _dbg_info; // allocated by debugInfo() on demand
private:
   IndigoObject (const IndigoObject &);
};

class IndigoGross : public IndigoObject
{
public:
   IndigoGross ();
   virtual ~IndigoGross ();

   virtual void toString (Array<char> &str);

   Array<int> gross;
};

struct DLLEXPORT ProductEnumeratorParams
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

class DLLEXPORT Indigo
{
public:
   Indigo ();
   ~Indigo ();

   Array<char> error_message;
   INDIGO_ERROR_HANDLER error_handler;
   void  *error_handler_context;

   IndigoObject & getObject (int handle);
   int countObjects ();

   int addObject (IndigoObject *obj);

   void removeObject (int id);

   void removeAllObjects ();

   Array<char> tmp_string;
   float tmp_xyz[3];

   ProductEnumeratorParams rpe_params;
   MoleculeFingerprintParameters fp_params;
   PtrArray<TautomerRule> tautomer_rules;
   
   bool ignore_stereochemistry_errors;
   bool ignore_noncritical_query_features;
   bool treat_x_as_pseudoatom;
   bool skip_3d_chirality;

   bool deconvolution_aromatization;
   bool deco_save_ap_bond_orders;
   bool deco_ignore_errors;

   int  molfile_saving_mode; // MolfileSaver::MODE_***, default is zero
   bool molfile_saving_no_chiral;
   bool molfile_saving_skip_date;

   bool smiles_saving_write_name;

   Encoding filename_encoding;

   bool embedding_edges_uniqueness, find_unique_embeddings;
   int max_embeddings;

   int layout_max_iterations; // default is zero -- no limit

   int aam_cancellation_timeout; //default is zero - no timeout

   int cancellation_timeout; // default is 0 seconds - no timeout
   TimeoutCancellationHandler timeout_cancellation_handler;

   void resetCancellationHandler ();

   void initMolfileSaver (MolfileSaver &saver);
   void initRxnfileSaver (RxnfileSaver &saver);

   bool preserve_ordering_in_serialize;

   AromaticityOptions arom_options;
   // This option is moved out of arom_options because it should be used only in indigoDearomatize method
   bool unique_dearomatization; 

protected:

   RedBlackMap<int, IndigoObject *> _objects;

   int    _next_id;
   OsLock _objects_lock;
};



#define INDIGO_BEGIN { Indigo &self = indigoGetInstance(); \
      try { self.error_message.clear(); self.resetCancellationHandler(); 

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
         self.error_message.readLine(ex.message(), true); \
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

class DLLEXPORT IndigoError : public Exception
{
public:
   explicit IndigoError (const char *format, ...);
   IndigoError (const IndigoError &);
private:
};

class _IndigoBasicOptionsHandlersSetter
{
public:
   _IndigoBasicOptionsHandlersSetter ();
   ~_IndigoBasicOptionsHandlersSetter ();
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
