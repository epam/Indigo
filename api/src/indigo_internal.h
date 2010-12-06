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

#include "base_cpp/tlscont.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj.h"
#include "base_cpp/io_base.h"

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molfile_saver.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_product_enumerator.h"
#include "reaction/reaction_highlighting.h"
#include "molecule/molecule_substructure_matcher.h"
#include "option_manager.h"
#include "molecule/molecule_fingerprint.h"

using namespace indigo;

namespace indigo
{
class Output;
class Scanner;
class SdfLoader;
class RdfLoader;
}

class IndigoAtom;
class IndigoBond;
class IndigoRGroup;
class IndigoArray;
class IndigoDeconvolution;
class IndigoFingerprint;

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

   DLLEXPORT IndigoBond & asBond ();
   virtual DLLEXPORT IndigoRGroup & getRGroup ();
   virtual DLLEXPORT int getIndex ();

   virtual DLLEXPORT IndigoObject * next ();
   
   virtual DLLEXPORT bool hasNext ();

   virtual DLLEXPORT IndigoArray & asArray ();
   virtual DLLEXPORT IndigoFingerprint & asFingerprint ();
   
   bool DLLEXPORT isBaseMolecule ();
   bool DLLEXPORT isBaseReaction ();
   bool DLLEXPORT isAtom ();

   void DLLEXPORT copyProperties (RedBlackStringObjMap< Array<char> > &other);

protected:
   Array<char> _dbg_info; // allocated by debugInfo() on demand
};

class IndigoBaseMolecule : public IndigoObject
{
public:
   DLLEXPORT explicit IndigoBaseMolecule (int type_);

   DLLEXPORT virtual ~IndigoBaseMolecule ();

   DLLEXPORT virtual GraphHighlighting * getMoleculeHighlighting ();
   DLLEXPORT virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   DLLEXPORT const char * debugInfo ();

   GraphHighlighting highlighting;

   RedBlackStringObjMap< Array<char> > properties;
};

class IndigoMolecule : public IndigoBaseMolecule
{
public:
   DLLEXPORT IndigoMolecule ();
   
   DLLEXPORT virtual ~IndigoMolecule ();

   DLLEXPORT virtual BaseMolecule & getBaseMolecule ();
   DLLEXPORT virtual Molecule & getMolecule ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT const char * debugInfo ();

   DLLEXPORT virtual IndigoObject * clone ();

   Molecule mol;
};

class IndigoQueryMolecule : public IndigoBaseMolecule
{
public:
   DLLEXPORT IndigoQueryMolecule ();

   DLLEXPORT virtual ~IndigoQueryMolecule ();

   DLLEXPORT virtual BaseMolecule & getBaseMolecule ();
   DLLEXPORT virtual QueryMolecule & getQueryMolecule ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT const char * debugInfo ();

   DLLEXPORT virtual IndigoObject * clone ();

   QueryMolecule qmol;
};

class IndigoAtom : public IndigoObject
{
public:
   IndigoAtom (BaseMolecule &mol_, int idx_);
   virtual ~IndigoAtom ();

   DLLEXPORT static IndigoAtom & cast (IndigoObject &obj);

   BaseMolecule *mol;
   int idx;

   virtual int getIndex ();
};

class IndigoRGroup : public IndigoObject
{
public:
   IndigoRGroup ();
   virtual ~IndigoRGroup ();

   virtual IndigoRGroup & getRGroup ();
   virtual int getIndex ();

   QueryMolecule *mol;
   int idx;
};

class IndigoRGroupFragment : public IndigoObject
{
public:
   IndigoRGroupFragment (IndigoRGroup &rgp, int idx);
   IndigoRGroupFragment (QueryMolecule *mol, int rgroup_idx, int fragment_idx);
   
   virtual ~IndigoRGroupFragment ();

   virtual QueryMolecule & getQueryMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   virtual int getIndex ();

   IndigoRGroup rgroup;
   int frag_idx;
};

class IndigoBond : public IndigoObject
{
public:
   IndigoBond (BaseMolecule &mol_, int idx_);
   virtual ~IndigoBond ();

   DLLEXPORT static IndigoBond & cast (IndigoObject &obj);

   BaseMolecule *mol;
   int idx;

   virtual int getIndex ();
};

class IndigoBaseReaction : public IndigoObject
{
public:
   explicit IndigoBaseReaction (int type_);

   virtual ~IndigoBaseReaction ();

   virtual ReactionHighlighting * getReactionHighlighting ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   ReactionHighlighting highlighting;
   RedBlackStringObjMap< Array<char> > properties;
};

class IndigoReaction : public IndigoBaseReaction
{
public:
   DLLEXPORT IndigoReaction ();
   DLLEXPORT virtual ~IndigoReaction ();

   DLLEXPORT virtual BaseReaction & getBaseReaction ();
   DLLEXPORT virtual Reaction & getReaction ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT virtual IndigoObject * clone();

   Reaction rxn;
};

class IndigoQueryReaction : public IndigoBaseReaction
{
public:
   DLLEXPORT IndigoQueryReaction ();
   DLLEXPORT virtual ~IndigoQueryReaction ();

   DLLEXPORT virtual BaseReaction & getBaseReaction ();
   DLLEXPORT virtual QueryReaction & getQueryReaction ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT virtual IndigoObject * clone();
   
   QueryReaction rxn;
};

class IndigoGross : public IndigoObject
{
public:
   IndigoGross ();
   virtual ~IndigoGross ();

   virtual void toString (Array<char> &str);

   Array<int> gross;
};

class IndigoReactionMolecule : public IndigoObject
{
public:
   IndigoReactionMolecule (BaseReaction &reaction, ReactionHighlighting *highlighting, int index);
   virtual ~IndigoReactionMolecule ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual Molecule & getMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting ();
   virtual int getIndex ();

   BaseReaction &rxn;
   ReactionHighlighting *hl;
   int idx;
};

class IndigoReactionIter : public IndigoObject
{
public:
   enum
   {
      REACTANTS,
      PRODUCTS,
      MOLECULES
   };

   IndigoReactionIter (BaseReaction &rxn, ReactionHighlighting *hl, int subtype);
   virtual ~IndigoReactionIter ();
   
   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _begin ();
   int _end ();
   int _next (int i);

   int _subtype;
   BaseReaction &_rxn;
   ReactionHighlighting *_hl;
   int _idx;
};

class IndigoAtomsIter : public IndigoObject
{
public:
   enum
   {
      ALL,
      PSEUDO,
      RSITE,
      STEREOCENTER
   };

   IndigoAtomsIter (BaseMolecule *molecule, int type);

   virtual ~IndigoAtomsIter ();
   
   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _shift (int idx);

   int _type;
   int _idx;
   BaseMolecule *_mol;
};

class IndigoBondsIter : public IndigoObject
{
public:
   IndigoBondsIter (BaseMolecule *molecule);

   virtual ~IndigoBondsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _idx;
   BaseMolecule *_mol;
};

class IndigoAtomNeighbor : public IndigoAtom
{
public:
   explicit IndigoAtomNeighbor (BaseMolecule &mol_, int atom_idx, int bond_idx);
   virtual ~IndigoAtomNeighbor ();

   int bond_idx;
};

class IndigoAtomNeighborsIter : public IndigoObject
{
public:
   IndigoAtomNeighborsIter (BaseMolecule *molecule, int atom_idx);

   virtual ~IndigoAtomNeighborsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _atom_idx;
   int _nei_idx;
   BaseMolecule *_mol;
};

class IndigoRGroupsIter : public IndigoObject
{
public:
   IndigoRGroupsIter (QueryMolecule *mol);

   virtual ~IndigoRGroupsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   QueryMolecule *_mol;
   int _idx;
};

class IndigoRGroupFragmentsIter : public IndigoObject
{
public:
   IndigoRGroupFragmentsIter (IndigoRGroup &rgroup);
   virtual ~IndigoRGroupFragmentsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   QueryMolecule *_mol;
   int _rgroup_idx;
   int _frag_idx;
};

// Query to the target match instance
class IndigoMoleculeSubstructureMatch : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatch (Molecule &target, QueryMolecule &query);
   virtual ~IndigoMoleculeSubstructureMatch ();

   DLLEXPORT const char * debugInfo ();

   GraphHighlighting highlighting;
   Array<int> query_atom_mapping;
   Molecule &target;
   QueryMolecule &query;
};

// Iterator for all possible matches
class IndigoMoleculeSubstructureMatchIter : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatchIter (Molecule &target, QueryMolecule &query, Molecule &original_target);

   virtual ~IndigoMoleculeSubstructureMatchIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   int countMatches (int max_embeddings);

   DLLEXPORT const char * debugInfo ();

   MoleculeSubstructureMatcher matcher;
   MoleculeSubstructureMatcher::FragmentMatchCache fmcache;
   GraphHighlighting highlighting;
   Molecule &target, &original_target;
   QueryMolecule &query;

   Array<int> mapping;

private:
   bool _initialized, _found, _need_find;
};

// Matcher class for matching queries on a specified target molecule
class IndigoMoleculeSubstructureMatcher : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatcher (Molecule &target);

   virtual ~IndigoMoleculeSubstructureMatcher ();

   IndigoMoleculeSubstructureMatchIter* iterateQueryMatches (QueryMolecule &query, 
      bool embedding_edges_uniqueness);

   DLLEXPORT const char * debugInfo ();

   Molecule &target;

private:
   Molecule _target_arom_h_unfolded, _target_arom;
   Array<int> _mapping_arom_h_unfolded, _mapping_arom;
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
