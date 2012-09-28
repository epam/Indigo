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

#ifndef __elements_h__
#define __elements_h__

#include "base_cpp/tlscont.h"
#include "base_cpp/red_black.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Scanner;

enum
{
   ELEM_MIN = 1,
   ELEM_H = 1,
   ELEM_He,
   ELEM_Li,
   ELEM_Be,
   ELEM_B ,
   ELEM_C ,
   ELEM_N ,
   ELEM_O ,
   ELEM_F ,
   ELEM_Ne,
   ELEM_Na,
   ELEM_Mg,
   ELEM_Al,
   ELEM_Si,
   ELEM_P ,
   ELEM_S ,
   ELEM_Cl,
   ELEM_Ar,
   ELEM_K,
   ELEM_Ca,
   ELEM_Sc,
   ELEM_Ti,
   ELEM_V,
   ELEM_Cr,
   ELEM_Mn,
   ELEM_Fe,
   ELEM_Co,
   ELEM_Ni,
   ELEM_Cu,
   ELEM_Zn,
   ELEM_Ga,
   ELEM_Ge,
   ELEM_As,
   ELEM_Se,
   ELEM_Br,
   ELEM_Kr,
   ELEM_Rb,
   ELEM_Sr,
   ELEM_Y,
   ELEM_Zr,
   ELEM_Nb,
   ELEM_Mo,
   ELEM_Tc,
   ELEM_Ru,
   ELEM_Rh,
   ELEM_Pd,
   ELEM_Ag,
   ELEM_Cd,
   ELEM_In,
   ELEM_Sn,
   ELEM_Sb,
   ELEM_Te,
   ELEM_I,
   ELEM_Xe,
   ELEM_Cs,
   ELEM_Ba,
   ELEM_La,
   ELEM_Ce,
   ELEM_Pr,
   ELEM_Nd,
   ELEM_Pm,
   ELEM_Sm,
   ELEM_Eu,
   ELEM_Gd,
   ELEM_Tb,
   ELEM_Dy,
   ELEM_Ho,
   ELEM_Er,
   ELEM_Tm,
   ELEM_Yb,
   ELEM_Lu,
   ELEM_Hf,
   ELEM_Ta,
   ELEM_W,
   ELEM_Re,
   ELEM_Os,
   ELEM_Ir,
   ELEM_Pt,
   ELEM_Au,
   ELEM_Hg,
   ELEM_Tl,
   ELEM_Pb,
   ELEM_Bi,
   ELEM_Po,
   ELEM_At,
   ELEM_Rn,
   ELEM_Fr,
   ELEM_Ra,
   ELEM_Ac,
   ELEM_Th,
   ELEM_Pa,
   ELEM_U,
   ELEM_Np,
   ELEM_Pu,
   ELEM_Am,
   ELEM_Cm,
   ELEM_Bk,
   ELEM_Cf,
   ELEM_Es,
   ELEM_Fm,
   ELEM_Md,
   ELEM_No,
   ELEM_Lr,
   ELEM_Rf,
   ELEM_MAX,

   ELEM_PSEUDO, // pseudoatom
   ELEM_RSITE  // 'R' atom
};

enum
{
   RADICAL_SINGLET = 1,
   RADICAL_DOUBLET = 2,
   RADICAL_TRIPLET = 3
};

class DLLEXPORT Element
{
public:
   DECL_ERROR;

   static const char * toString (int element);
   static int fromString (const char *name);
   static int fromString2 (const char *name);
   static int fromChar (char c);
   static int fromTwoChars (char c1, char c2);
   static int fromTwoChars2 (char c1, char c2);
   
   static int radicalElectrons (int radical);
   static int radicalOrbitals  (int radical);

   static bool calcValence (int elem, int charge, int radical, int conn, int &valence, int &hyd, bool to_throw);
   static int  calcValenceOfAromaticAtom (int elem, int charge, int n_arom, int min_conn);
   static int  calcValenceMinusHyd (int elem, int charge, int radical, int conn);

   // Calculate maximum number of single bonds that 
   // can be attached to specified atom.
   static int getMaximumConnectivity (int elem, int charge, 
                                      int radical, bool use_d_orbital);
   static int orbitals (int elem, bool use_d_orbital);
   static int electrons (int elem, int charge);

   static int group (int element);
   static int period (int period);
   static int read (Scanner &scanner);

   static bool isHalogen (int element);

   static int   getDefaultIsotope       (int element);
   static float getRelativeIsotopicMass (int element, int isotope);
   static float getStandardAtomicWeight (int element);
   static bool  getIsotopicComposition  (int element, int isotope, float &res);
   static void  getMinMaxIsotopeIndex   (int element, int &min, int &max);

   static bool  canBeAromatic (int element);

   static Array<int> & tautomerHeteroatoms ();

private:

   Element ();
   
   static Element _instance;

   void _initAllPeriodic ();
   void _initPeriodic (int element, const char *name, int period, int group);

   void _setStandardAtomicWeightIndex (int element, int index);
   void _addElementIsotope            (int element, int isotope, 
                                       float mass, float isotopic_composition);
   void _initAllIsotopes ();
   void _initDefaultIsotopes ();

   void _initAromatic ();

   RedBlackStringMap<int> _map;
   Array<int> _halogens;
   Array<int> _tau_heteroatoms;  // Appear in tautomer chains

   // Per-element physical parameters
   struct _Parameters
   {
      char name[3];
      
      int group;
      int period;
      
      int natural_isotope_index; // Can be ElementIsotope::NATURAL or anything else 
      // Isotope with highest mole fraction
      int default_isotope;
      // Minimum and maximum isotope index
      int min_isotope_index, max_isotope_index;

      bool can_be_aromatic;
   };
   
   Array<_Parameters> _element_parameters;

   // Isotopes mass key
   struct DLLEXPORT _IsotopeKey
   {
      int element;
      int isotope; // Can be equal to NATURAL

      // Pseudoisotope value for weighted sum of the
      // atomic masses of all the isotopes for specified element
      enum { NATURAL = -1, UNINITIALIZED = -2 };

      _IsotopeKey (int element, int isotope);

      bool operator< (const _IsotopeKey &right) const;
   };

   struct _IsotopeValue
   {
      _IsotopeValue (float mass, float isotopic_composition);

      // Isotope mass
      float mass;
      // Mole fraction of the various isotopes
      float isotopic_composition;
   };
   RedBlackMap<_IsotopeKey, _IsotopeValue> _isotope_parameters_map;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
