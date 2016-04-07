/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __molecule_sgroups__
#define __molecule_sgroups__

#include "math/algebra.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_pool.h"
#include "base_cpp/ptr_pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

class DLLEXPORT SGroup
{
public:   
   enum
   {
      SG_TYPE_GEN = 0,
      SG_TYPE_DAT,
      SG_TYPE_SUP,
      SG_TYPE_SRU,
      SG_TYPE_MUL,
      SG_TYPE_MON,
      SG_TYPE_MER,
      SG_TYPE_COP,
      SG_TYPE_CRO,
      SG_TYPE_MOD,
      SG_TYPE_GRA,
      SG_TYPE_COM,
      SG_TYPE_MIX,
      SG_TYPE_FOR,
      SG_TYPE_ANY
   };

   enum
   {
      SG_SUBTYPE_ALT = 1,
      SG_SUBTYPE_RAN,
      SG_SUBTYPE_BLO
   };

   enum
   {
      HEAD_TO_HEAD = 1,
      HEAD_TO_TAIL,
      EITHER
   };

   enum
   {
      SG_TYPE = 1,
      SG_CLASS,
      SG_LABEL,
      SG_DISPLAY_OPTION,
      SG_BRACKET_STYLE,
      SG_DATA,
      SG_DATA_NAME,
      SG_DATA_TYPE,
      SG_DATA_DESCRIPTION,
      SG_DATA_DISPLAY,
      SG_DATA_LOCATION,
      SG_DATA_TAG,
      SG_QUERY_CODE,
      SG_QUERY_OPER,
      SG_PARENT,
      SG_CHILD,
      SG_ATOMS,
      SG_BONDS
   };

   struct SgType
   {
      const int  int_type;
      const char *str_type;
   };
  
   SGroup ();
   virtual ~SGroup ();

   int    sgroup_type;    // group type, represnted with STY in Molfile format
   int    sgroup_subtype; // group subtype, represnted with SST in Molfile format
   int    original_group; // original group number
   int    parent_group;   // parent group number; represented with SPL in Molfile format 

   Array<int> atoms; // represented with SAL in Molfile format
   Array<int> bonds; // represented with SBL in Molfile format

   int    brk_style; // represented with SBT in Molfile format
   Array<Vec2f[2]> brackets; // represented with SDI in Molfile format

   static const char * typeToString(int sg_type);
   static int getType(const char * sg_type);

private:
   SGroup (const SGroup &);
};

class DLLEXPORT DataSGroup : public SGroup
{
public:
   DataSGroup ();
   virtual ~DataSGroup ();

   Array<char> description; // SDT in Molfile format (filed units or format)
   Array<char> name;        // SDT in Molfile format (field name)
   Array<char> type;        // SDT in Molfile format (field type)
   Array<char> querycode;   // SDT in Molfile format (query code)
   Array<char> queryoper;   // SDT in Molfile format (query operator)
   Array<char> data;        // SCD/SED in Molfile format (field data)
   Vec2f       display_pos; // SDD in Molfile format
   bool        detached;    // or attached
   bool        relative;    // or absolute
   bool        display_units;
   int         num_chars;   // number of characters 
   int         dasp_pos;
   char        tag;         // tag  
private:
   DataSGroup (const DataSGroup &);
};

class DLLEXPORT Superatom : public SGroup
{
public:
   Superatom ();
   virtual ~Superatom ();

   Array<char> subscript; // SMT in Molfile format
   Array<char> sa_class;  // SCL in Molfile format
   int   contracted;      // display option (-1 if undefined, 0 - expanded, 1 - contracted)
                          // SDS in Molfile format

   struct _AttachmentPoint
   {
      int  aidx;
      int  lvidx;
      Array<char> apid;
   };
   ObjPool<_AttachmentPoint> attachment_points;  // SAP in Molfile format

   struct _BondConnection
   {
      int   bond_idx;
      Vec2f bond_dir;
   };
   Array<_BondConnection> bond_connections;  // SBV in Molfile format

private:
   Superatom (const Superatom &);
};

class DLLEXPORT RepeatingUnit : public SGroup
{
public:
   RepeatingUnit ();
   virtual ~RepeatingUnit ();

   int connectivity;
   Array<char> subscript; // SMT in Molfile format
private:
   RepeatingUnit (const RepeatingUnit &);
};

class DLLEXPORT MultipleGroup : public SGroup
{
public:
   MultipleGroup ();
   virtual ~MultipleGroup ();

   Array<int> parent_atoms;
   int multiplier;
private:
   MultipleGroup (const MultipleGroup &);
};

class DLLEXPORT MoleculeSGroups
{
public:

   MoleculeSGroups ();
   ~MoleculeSGroups ();

   DECL_ERROR;

   int addSGroup (const char * sg_type);
   int addSGroup (int sg_type);
   SGroup &getSGroup  (int idx);
   SGroup &getSGroup  (int idx, int sg_type);
   int getSGroupCount ();
   int getSGroupCount (int sg_type);
   bool isPolimer();

   void remove(int idx);
   void clear ();
   void clear (int sg_type);

   int begin();
   int end();
   int next(int i);

   enum PropertyTypes { PROPERTY_INT, PROPERTY_BOOL, PROPERTY_STRING, PROPERTY_INT_ARRAY };
   static void parseCondition (const char * property, const char * value, int &s_property, int &s_type, int &s_int,
                               Array<int> &s_indices);

   void findSGroups (const char * property, const char * value, Array<int> &sgs);
   void findSGroups (int property, int value, Array<int> &sgs);
   void findSGroups (int property, const char *value, Array<int> &sgs);
   void findSGroups (int property, Array<int> &value, Array<int> &sgs);

   void registerUnfoldedHydrogen(int idx, int new_h_idx);

protected:
   PtrPool<SGroup> _sgroups;

private:
   int _findSGroupById (int id);
   bool _cmpIndices (Array<int> &t_inds, Array<int> &q_inds);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
