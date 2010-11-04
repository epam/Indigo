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

class Output;
class Scanner;
class SdfLoader;
class RdfLoader;
class IndigoAtom;
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
      MOLECULE_SUBSTRUCTURE_MATCHER,
      SCAFFOLD,
      DECONVOLUTION,
      DECONVOLUTION_ELEM,
      DECONVOLUTION_ITER,
      PROPERTIES_ITER,
      PROPERTY,
      FINGERPRINT
   };

   int type;

   const char * debugInfo ();

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

   DLLEXPORT Scanner & getScanner ();
   DLLEXPORT Output  & getOutput ();

   virtual DLLEXPORT const char * getName ();

   virtual DLLEXPORT IndigoAtom & getAtom ();
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
   Array<char> _dbg_info;
};

class IndigoBaseMolecule : public IndigoObject
{
public:
   explicit IndigoBaseMolecule (int type_);

   virtual ~IndigoBaseMolecule ();

   virtual GraphHighlighting * getMoleculeHighlighting ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   GraphHighlighting highlighting;

   RedBlackStringObjMap< Array<char> > properties;
};

class IndigoMolecule : public IndigoBaseMolecule
{
public:
   IndigoMolecule ();
   
   virtual ~IndigoMolecule ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual Molecule & getMolecule ();
   virtual const char * getName ();

   virtual IndigoObject * clone ();

   Molecule mol;
};

class IndigoQueryMolecule : public IndigoBaseMolecule
{
public:
   IndigoQueryMolecule ();

   virtual ~IndigoQueryMolecule ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual const char * getName ();

   virtual IndigoObject * clone ();

   QueryMolecule qmol;
};

class IndigoAtom : public IndigoObject
{
public:
   IndigoAtom (BaseMolecule &mol_, int idx_);
   virtual ~IndigoAtom ();

   BaseMolecule *mol;
   int idx;

   virtual IndigoAtom & getAtom ();
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
   IndigoReaction ();
   virtual ~IndigoReaction ();

   virtual BaseReaction & getBaseReaction ();
   virtual Reaction & getReaction ();
   virtual const char * getName ();

   virtual IndigoObject * clone();

   Reaction rxn;
};

class IndigoQueryReaction : public IndigoBaseReaction
{
public:
   IndigoQueryReaction ();
   virtual ~IndigoQueryReaction ();

   virtual BaseReaction & getBaseReaction ();
   virtual QueryReaction & getQueryReaction ();
   virtual const char * getName ();

   virtual IndigoObject * clone();
   
   QueryReaction rxn;
};

class IndigoScanner : public IndigoObject
{
public:
   IndigoScanner (Scanner *scanner);
   IndigoScanner (const char *str);
   IndigoScanner (const char *buf, int size);

   virtual ~IndigoScanner ();

   Scanner *ptr;
protected:
   Array<char> _buf;
};

class IndigoOutput : public IndigoObject
{
public:
   IndigoOutput ();
   IndigoOutput (Output *output);
   virtual ~IndigoOutput ();

   virtual void toString (Array<char> &str);

   virtual Output & getOutput ();

   Output *ptr;
protected:
   bool        _own_buf;
   Array<char> _buf;
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

class IndigoRdfData : public IndigoObject
{
public:
   IndigoRdfData (int type, Array<char> &data, int index, int offset);
   IndigoRdfData (int type, Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                  int index, int offset);
   virtual ~IndigoRdfData ();

   Array<char> & getRawData ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   virtual int getIndex ();
   int tell ();

protected:
   Array<char> _data;
   RedBlackStringObjMap< Array<char> > _properties;
   
   bool _loaded;
   int _index;
   int _offset;
};

class IndigoRdfMolecule : public IndigoRdfData
{
public:
   IndigoRdfMolecule (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                      int index, int offset);
   virtual ~IndigoRdfMolecule ();

   virtual Molecule & getMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   GraphHighlighting * getMoleculeHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Molecule _mol;
   GraphHighlighting _highlighting;
};

class IndigoRdfReaction : public IndigoRdfData
{
public:
   IndigoRdfReaction (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                      int index, int offset);
   virtual ~IndigoRdfReaction ();

   virtual Reaction & getReaction ();
   virtual BaseReaction & getBaseReaction ();
   ReactionHighlighting * getReactionHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Reaction _rxn;
   ReactionHighlighting _highlighting;
};


class IndigoSdfLoader : public IndigoObject
{
public:
   IndigoSdfLoader (Scanner &scanner);
   IndigoSdfLoader (const char *filename);
   virtual ~IndigoSdfLoader ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   int tell ();

   GraphHighlighting highlighting;
   SdfLoader *sdf_loader;
   
protected:
   Scanner  *_own_scanner;
   int       _counter;
};

class IndigoRdfLoader : public IndigoObject
{
public:
   IndigoRdfLoader (Scanner &scanner);
   IndigoRdfLoader (const char *filename);
   virtual ~IndigoRdfLoader ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   int tell ();

   RdfLoader *rdf_loader;
protected:
   int       _counter;
   Scanner  *_own_scanner;
};

class IndigoSmilesMolecule : public IndigoRdfData
{
public:
   IndigoSmilesMolecule (Array<char> &smiles, int index, int offset);
   virtual ~IndigoSmilesMolecule ();

   virtual Molecule & getMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   GraphHighlighting * getMoleculeHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Molecule _mol;
   GraphHighlighting _highlighting;
};

class IndigoSmilesReaction : public IndigoRdfData
{
public:
   IndigoSmilesReaction (Array<char> &data, int index, int offset);
   virtual ~IndigoSmilesReaction ();

   virtual Reaction & getReaction ();
   virtual BaseReaction & getBaseReaction ();
   ReactionHighlighting * getReactionHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Reaction _rxn;
   ReactionHighlighting _highlighting;
};

class IndigoMultilineSmilesLoader : public IndigoObject
{
public:
   IndigoMultilineSmilesLoader (Scanner &scanner);
   IndigoMultilineSmilesLoader (const char *filename);
   virtual ~IndigoMultilineSmilesLoader ();

   int tell ();
   
   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   Scanner    *_scanner;
   Array<char> _str;
   bool      _own_scanner;
   int       _counter;
};

class IndigoAtomsIter : public IndigoObject
{
public:
   enum
   {
      ALL,
      PSEUDO,
      RSITE
   };

   IndigoAtomsIter (BaseMolecule *molecule, int type);

   virtual ~IndigoAtomsIter ();
   
   virtual IndigoObject * next ();
   virtual bool hasNext ();
   virtual int getIndex ();

protected:

   int _shift (int idx);

   int _type;
   int _idx;
   BaseMolecule *_mol;
};

class IndigoRGroupsIter : public IndigoObject
{
public:
   IndigoRGroupsIter (QueryMolecule *mol);

   virtual ~IndigoRGroupsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();
   virtual int getIndex ();

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
   virtual int getIndex ();

protected:
   QueryMolecule *_mol;
   int _rgroup_idx;
   int _frag_idx;
};

class IndigoArray : public IndigoObject
{
public:
   IndigoArray ();

   virtual ~IndigoArray ();

   virtual IndigoObject * clone ();
   virtual IndigoArray & asArray ();

   PtrArray<IndigoObject> objects;
};

class IndigoArrayElement : public IndigoObject
{
public:
   IndigoArrayElement (IndigoArray &arr, int idx_);
   virtual ~IndigoArrayElement ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual Molecule & getMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting();

   virtual BaseReaction & getBaseReaction ();
   virtual Reaction & getReaction ();
   virtual ReactionHighlighting * getReactionHighlighting();

   virtual IndigoObject * clone ();

   virtual const char * getName ();

   virtual IndigoArray & asArray ();
   virtual IndigoFingerprint & asFingerprint ();

   virtual int getIndex ();

   IndigoArray *array;
   int idx;
};

class IndigoArrayIter : public IndigoObject
{
public:
   IndigoArrayIter (IndigoArray &arr);
   virtual ~IndigoArrayIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();
   virtual int getIndex ();
protected:
   IndigoArray *_arr;
   int _idx;
};

class IndigoMoleculeSubstructureMatcher : public IndigoObject
{
public:
   IndigoMoleculeSubstructureMatcher (Molecule &target_);

   virtual ~IndigoMoleculeSubstructureMatcher ();

   MoleculeSubstructureMatcher matcher;
   MoleculeSubstructureMatcher::FragmentMatchCache fmcache;
   GraphHighlighting highlighting;
   Array<int> target_bond_orders;

   Molecule &target;
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

class IndigoScaffold : public IndigoObject
{
public:
   IndigoScaffold ();
   virtual ~IndigoScaffold ();

   void extractScaffold ();

   virtual Molecule & getMolecule();
   virtual BaseMolecule & getBaseMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting ();

   Molecule           max_scaffold;
   ObjArray<Molecule> all_scaffolds;
};

class IndigoDeconvolution : public IndigoObject {
private:
   enum {
      SHIFT_IDX = 2
   };
public:
   class Item {
   public:
      Item(Molecule& mol):mol_in(mol) {}

      Molecule & mol_in;
      Molecule   mol_out;
      GraphHighlighting highlight;
      QueryMolecule rgroup_mol;
      RedBlackStringObjMap< Array<char> > properties;
   private:
      Item(const Item&);
   };
   IndigoDeconvolution(bool aromatize);
   virtual ~IndigoDeconvolution();

   void addMolecule(Molecule& mol, RedBlackStringObjMap< Array<char> >* props);

   void makeRGroups (Molecule& scaffold);

   Molecule& getDecomposedScaffold();
   ObjArray<Item>& getItems ();
   
   int flags;

   static bool matchBonds (Graph &g1, Graph &g2, int i, int j, void* userdata);
   static bool matchAtoms (Graph &g1, Graph &g2, const int *core_sub, int i, int j, void* userdata);

   ObjArray<Molecule> scaffolds;

   int (*cbEmbedding) (const int *sub_vert_map, const int *sub_edge_map, const void* info, void* userdata);
   void *embeddingUserdata;

  
private:
   class EmbContext {
   public:
       EmbContext(int flag):flags(flag) {}
       Array<int> visitedAtoms;
       Array<int> lastMapping;
       Array<int> lastInvMapping;
       ObjArray< Array<int> > attachmentOrder;
       ObjArray< Array<int> > attachmentIndex;
       int flags;

       int getRgroupNumber() const { return attachmentIndex.size()-1;}
       
       void renumber(Array<int>& map, Array<int>& inv_map);
   private:
       EmbContext(const EmbContext&); //no implicit copy
   };
   void _makeRGroup (Item& elem);
   void _createRgroups(Molecule& molecule_set, QueryMolecule& r_molecule, EmbContext& emb_context);
   void _parseOptions(const char* options);
   int _findOrAddFullRGroup(Array<int>& att_order, Array<int>& att_idx, QueryMolecule& qmol, Array<int>& map);
   
   static int _rGroupsEmbedding(Graph &g1, Graph &g2, int *core1, int *core2, void *userdata);

   bool _aromatic;
   
   Molecule _scaffold;
   Molecule _fullScaffold;
   ObjArray<Item> _deconvolutionItems;

   DEF_ERROR("R-Group deconvolution");
};

class IndigoDeconvolutionIter : public IndigoObject {
public:

   IndigoDeconvolutionIter(ObjArray<IndigoDeconvolution::Item>& items);
   virtual ~IndigoDeconvolutionIter();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   int _index;
   ObjArray<IndigoDeconvolution::Item>& _items;
};

class IndigoDeconvolutionElem : public IndigoObject
{
public:
   IndigoDeconvolutionElem (IndigoDeconvolution::Item &item, int index);
   ~IndigoDeconvolutionElem ();

   virtual int getIndex ();
   
   IndigoDeconvolution::Item &item;
   int idx;
};

class IndigoProperty : public IndigoObject
{
public:
   IndigoProperty (RedBlackStringObjMap< Array<char> > &props, int idx);
   virtual ~IndigoProperty ();

   virtual const char * getName ();
   virtual int getIndex ();

protected:
   RedBlackStringObjMap< Array<char> > &_props;
   int _idx;
};

class IndigoPropertiesIter : public IndigoObject
{
public:
   IndigoPropertiesIter (RedBlackStringObjMap< Array<char> > &props);
   virtual ~IndigoPropertiesIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();
   
protected:
   RedBlackStringObjMap< Array<char> > &_props;
   int _idx;
};

class IndigoFingerprint : public IndigoObject
{
public:
   IndigoFingerprint ();
   virtual ~IndigoFingerprint ();

   virtual DLLEXPORT void toString (Array<char> &str);
   virtual DLLEXPORT void toBuffer (Array<char> &buf);
   virtual IndigoFingerprint & asFingerprint ();

   Array<byte> bytes;
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

   Array<char> tmp_string;

   ProductEnumeratorParams rpe_params;

   MoleculeFingerprintParameters fp_params;
   
   bool ignore_stereochemistry_errors;
   bool treat_x_as_pseudoatom;

   bool deconvolution_aromatization;

   int  molfile_saving_mode; // MolfileSaver::MODE_***, default is zero

protected:

   RedBlackMap<int, IndigoObject *> _objects;

   int    _next_id;
   OsLock _objects_lock;
};



#define INDIGO_BEGIN { TL_GET2(Indigo, self, indigo_self); \
      try { self.error_message.clear();

#define INDIGO_END(success, fail) } \
      catch (Exception &ex)         \
      {                             \
          self.error_message.readString(ex.message(), true); \
          if (self.error_handler != 0)                       \
             self.error_handler(ex.message(),                \
                                self.error_handler_context); \
          return fail;                                       \
      }                                                      \
      return success; }                                      

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

TL_DECL_EXT(Indigo, indigo_self);

DLLEXPORT Indigo & indigoGetInstance ();

class IndigoError : public Exception
{
public:
   explicit DLLEXPORT IndigoError (const char *format, ...);
   DLLEXPORT IndigoError (const IndigoError &);
private:
};

#endif
