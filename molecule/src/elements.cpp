/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"

using namespace indigo;

Element Element::_instance;

Element::Element ()
{
   _element_parameters.resize(ELEM_MAX);
   _element_parameters.zerofill();
   
   _initAllPeriodic();
   _initAllIsotopes();
   _initAromatic();

   _halogens.push(ELEM_F);
   _halogens.push(ELEM_Cl);
   _halogens.push(ELEM_Br);
   _halogens.push(ELEM_I);
   _halogens.push(ELEM_At);


}

void Element::_initPeriodic (int element, const char *name, int period, int group)
{
   _Parameters &parameters = _element_parameters[element];
   
   strncpy(parameters.name, name, 3);
   parameters.group = group;
   parameters.period = period;
   
   _map.insert(name, element);
}

int Element::radicalElectrons (int radical)
{
   if (radical == RADICAL_DOUPLET)
      return 1;
   if (radical == RADICAL_SINGLET || radical == RADICAL_TRIPLET)
      return 2;
   return 0;
}

int Element::radicalOrbitals  (int radical)
{
   if (radical != 0)
      return 1;
   return 0;
}

void Element::_initAllPeriodic ()
{
   #define INIT(elem, period, group) _initPeriodic(ELEM_##elem, #elem, period, group)

   INIT(H,  1, 1);
   INIT(He, 1, 8);
   INIT(Li, 2, 1);
   INIT(Be, 2, 2);
   INIT(B,  2, 3);
   INIT(C,  2, 4);
   INIT(N,  2, 5);
   INIT(O,  2, 6);
   INIT(F,  2, 7);
   INIT(Ne, 2, 8);
   INIT(Na, 3, 1);
   INIT(Mg, 3, 2);
   INIT(Al, 3, 3);
   INIT(Si, 3, 4);
   INIT(P,  3, 5);
   INIT(S,  3, 6);
   INIT(Cl, 3, 7);
   INIT(Ar, 3, 8);
   INIT(K,  4, 1);
   INIT(Ca, 4, 2);
   INIT(Sc, 4, 3);
   INIT(Ti, 4, 4);
   INIT(V,  4, 5);
   INIT(Cr, 4, 6);
   INIT(Mn, 4, 7);
   INIT(Fe, 4, 8);
   INIT(Co, 4, 8);
   INIT(Ni, 4, 8);
   INIT(Cu, 4, 1);
   INIT(Zn, 4, 2);
   INIT(Ga, 4, 3);
   INIT(Ge, 4, 4);
   INIT(As, 4, 5);
   INIT(Se, 4, 6);
   INIT(Br, 4, 7);
   INIT(Kr, 4, 8);
   INIT(Rb, 5, 1);
   INIT(Sr, 5, 2);
   INIT(Y,  5, 3);
   INIT(Zr, 5, 4);
   INIT(Nb, 5, 5);
   INIT(Mo, 5, 6);
   INIT(Tc, 5, 7);
   INIT(Ru, 5, 8);
   INIT(Rh, 5, 8);
   INIT(Pd, 5, 8);
   INIT(Ag, 5, 1);
   INIT(Cd, 5, 2);
   INIT(In, 5, 3);
   INIT(Sn, 5, 4);
   INIT(Sb, 5, 5);
   INIT(Te, 5, 6);
   INIT(I,  5, 7);
   INIT(Xe, 5, 8);
   INIT(Cs, 6, 1);
   INIT(Ba, 6, 2);
   INIT(La, 6, 3);
   INIT(Ce, 6, 3);
   INIT(Pr, 6, 3);
   INIT(Nd, 6, 3);
   INIT(Pm, 6, 3);
   INIT(Sm, 6, 3);
   INIT(Eu, 6, 3);
   INIT(Gd, 6, 3);
   INIT(Tb, 6, 3);
   INIT(Dy, 6, 3);
   INIT(Ho, 6, 3);
   INIT(Er, 6, 3);
   INIT(Tm, 6, 3);
   INIT(Yb, 6, 3);
   INIT(Lu, 6, 3);
   INIT(Hf, 6, 4);
   INIT(Ta, 6, 5);
   INIT(W,  6, 6);
   INIT(Re, 6, 7);
   INIT(Os, 6, 8);
   INIT(Ir, 6, 8);
   INIT(Pt, 6, 8);
   INIT(Au, 6, 1);
   INIT(Hg, 6, 2);
   INIT(Tl, 6, 3);
   INIT(Pb, 6, 4);
   INIT(Bi, 6, 5);
   INIT(Po, 6, 6);
   INIT(At, 6, 7);
   INIT(Rn, 6, 8);
   INIT(Fr, 7, 1);
   INIT(Ra, 7, 2);
   INIT(Ac, 7, 3);
   INIT(Th, 7, 3);
   INIT(Pa, 7, 3);
   INIT(U,  7, 3);
   INIT(Np, 7, 3);
   INIT(Pu, 7, 3);
   INIT(Am, 7, 3);
   INIT(Cm, 7, 3);
   INIT(Bk, 7, 3);
   INIT(Cf, 7, 3);
   INIT(Es, 7, 3);
   INIT(Fm, 7, 3);
   INIT(Md, 7, 3);
   INIT(No, 7, 3);
   INIT(Lr, 7, 3);
   INIT(Rf, 7, 3);
   #undef INIT
}

int Element::fromString (const char *name)
{
   int *value = _instance._map.at2(name);

   if (value == 0)
      throw Error("fromString(): element %s not supported", name);

   return *value;
}

int Element::fromString2 (const char *name)
{
   int *value = _instance._map.at2(name);

   if (value == 0)
      return -1;

   return *value;
}

int Element::fromChar (char c)
{
   char str[2] = {c, 0};

   return fromString(str);
}

int Element::fromTwoChars (char c1, char c2)
{
   char str[3] = {c1, c2, 0};

   return fromString(str);
}

int Element::fromTwoChars2 (char c1, char c2)
{
   char str[3] = {c1, c2, 0};

   return fromString2(str);
}

bool Element::isHalogen (int element)
{
   return _instance._halogens.find(element) >= 0;
}

const char * Element::toString (int element)
{
   if (element < 0 || element > ELEM_MAX)
      throw Error("bad element number: %d", element);

   return _instance._element_parameters[element].name;
}

int Element::calcValenceByCharge (int elem, int charge)
{
   if (elem == ELEM_Li || elem == ELEM_Na || elem == ELEM_K ||
       elem == ELEM_Rb || elem == ELEM_Cs || elem == ELEM_Fr)
      return 1;
   if (elem == ELEM_C)
      return 4;
   if (elem == ELEM_N)
      return (charge == 1 ? 4 : 3);
   if (elem == ELEM_O)
      return (charge >= 1 ? 3 : 2);
   return -1;
}

bool Element::calcValence (int elem, int charge, int radical, int conn, int &valence, int &hyd, bool to_throw)
{
   int groupno = Element::group(elem);
   int rad = radicalElectrons(radical);
   
   valence = conn;
   hyd = 0;
   
   if (groupno == 1)
   {
      if (elem == ELEM_Li || elem == ELEM_Na || elem == ELEM_K ||
          elem == ELEM_Rb || elem == ELEM_Cs || elem == ELEM_Fr)
      {
         valence = 1;
         hyd = 1 - rad - conn - abs(charge);
      }
   }
   else if (groupno == 3)
   {
      if (elem == ELEM_B || elem == ELEM_Al || elem == ELEM_Ga || elem == ELEM_In)
      {
         if (charge == -1)
         {
            valence = 4;
            hyd = 4 - rad - conn;
         }
         else
         {
            valence = 3;
            hyd = 3 - rad - conn - abs(charge);
         }
      }
      else if (elem == ELEM_Tl)
      {
         if (charge == -1)
         {
            if (rad + conn <= 2)
            {
               valence = 2;
               hyd = 2 - rad - conn;
            }
            else
            {
               valence = 4;
               hyd = 4 - rad - conn;
            }
         }
         else if (charge == -2)
         {
            if (rad + conn <= 3)
            {
               valence = 3;
               hyd = 3 - rad - conn;
            }
            else
            {
               valence = 5;
               hyd = 5 - rad - conn;
            }
         }
         else
         {
            if (rad + conn + abs(charge) <= 1)
            {
               valence = 1;
               hyd = 1 - rad - conn - abs(charge);
            }
            else
            {
               valence = 3;
               hyd = 3 - rad - conn - abs(charge);
            }
         }
      }      
   }
   else if (groupno == 4)
   {
      if (elem == ELEM_C || elem == ELEM_Si || elem == ELEM_Ge)
      {
         valence = 4;
         hyd = 4 - rad - conn - abs(charge);
      }
      else if (elem == ELEM_Sn || elem == ELEM_Pb)
      {
         if (conn + rad + abs(charge) <= 2)
         {
            valence = 2;
            hyd = 2 - rad - conn - abs(charge);
         }
         else
         {
            valence = 4;
            hyd = 4 - rad - conn - abs(charge);
         }
      }
   }
   else if (groupno == 5)
   {
      if (elem == ELEM_N || elem == ELEM_P)
      {
         if (charge == 1)
         {
            valence = 4;
            hyd = 4 - rad - conn;
         }
         else if (charge == 2)
         {
            valence = 3;
            hyd = 3 - rad - conn;
         }
         else
         {
            if (elem == ELEM_N || rad + conn + abs(charge) <= 3)
            {
               valence = 3;
               hyd = 3 - rad - conn - abs(charge);
            }
            else // ELEM_P && rad + conn + abs(charge) > 3
            {
               valence = 5;
               hyd = 5 - rad - conn - abs(charge);
            }
         }
      }
      else if (elem == ELEM_Bi || elem == ELEM_Sb || elem == ELEM_As)
      {
         if (charge == 1)
         {
            if (rad + conn <= 2 && elem != ELEM_As)
            {
               valence = 2;
               hyd = 2 - rad - conn;
            }
            else
            {
               valence = 4;
               hyd = 4 - rad - conn;
            }
         }
         else if (charge == 2)
         {
            valence = 3;
            hyd = 3 - rad - conn;
         }
         else
         {
            if (rad + conn <= 3)
            {
               valence = 3;
               hyd = 3 - rad - conn - abs(charge);
            }
            else
            {
               valence = 5;
               hyd = 5 - rad - conn - abs(charge);
            }
         }
      }
   }
   else if (groupno == 6)
   {
      if (elem == ELEM_O)
      {
         if (charge >= 1)
         {
            valence = 3;
            hyd = 3 - rad - conn;
         }
         else
         {
            valence = 2;
            hyd = 2 - rad - conn - abs(charge);
         }
      }
      else if (elem == ELEM_S || elem == ELEM_Se || elem == ELEM_Po)
      {
         if (charge == 1)
         {
            if (conn <= 3)
            {
               valence = 3;
               hyd = 3 - rad - conn;
            }
            else
            {
               valence = 5;
               hyd = 5 - rad - conn;
            }
         }
         else if (charge == -1)
         {
            if (conn + rad <= 1)
            {
               valence = 1;
               hyd = 1 - rad - conn;
            }
            else if (conn + rad <= 3)
            {
               valence = 3;
               hyd = 3 - rad - conn;
            }
            // no real examples for the other two cases, just following ISIS/Draw logic
            else if (conn + rad <= 5)
            {
               valence = 5;
               hyd = 5 - rad - conn;
            }
            else
            {
               valence = 7;
               hyd = 7 - rad - conn;
            }
         }
         else
         {
            if (conn + rad + abs(charge) <= 2)
            {
               valence = 2;
               hyd = 2 - rad - conn - abs(charge);
            }
            else if (conn + rad + abs(charge) <= 4)
            // See examples in PubChem
            // [S] : CID 16684216
            // [Se]: CID 5242252
            // [Po]: no example, just following ISIS/Draw logic here
            {
               valence = 4;
               hyd = 4 - rad - conn - abs(charge);
            }
            else
            // See examples in PubChem
            // [S] : CID 46937044
            // [Se]: CID 59786
            // [Po]: no example, just following ISIS/Draw logic here
            {
               valence = 6;
               hyd = 6 - rad - conn - abs(charge);
            }
         }
      }
      else if (elem == ELEM_Te)
      {
         if (charge == -1)
         {
            if (conn <= 2)
            {
               valence = 2;
               hyd = 2 - rad - conn - abs(charge);
            }
         }
         else if (charge == 0 || charge == 2)
         {
            if (conn <= 2)
            {
               valence = 2;
               hyd = 2 - rad - conn - abs(charge);
            }
            else if (conn <= 4)
            {
               valence = 4;
               hyd = 4 - rad - conn - abs(charge);
            }
            else if (charge == 0 && conn <= 6)
            {
               valence = 6;
               hyd = 6 - rad - conn - abs(charge);
            }
            else
               hyd = -1;
         }
      }
   }
   else if (groupno == 7)
   {
      if (elem == ELEM_F)
      {
         valence = 1;
         hyd = 1 - rad - conn - abs(charge);
      }
      else if (elem == ELEM_Cl || elem == ELEM_Br ||
               elem == ELEM_I  || elem == ELEM_At)
      {
         if (charge == 1)
         {
            if (conn <= 2)
            {
               valence = 2;
               hyd = 2 - rad - conn;
            }
            else if (conn == 3 || conn == 5 || conn >= 7)
               hyd = -1;
         }
         else if (charge == 0)
         {
            if (conn <= 1)
            {
               valence = 1;
               hyd = 1 - rad - conn;
            }
            // While the halogens can have valence 3, they can not have
            // hydrogens in that case.
            else if (conn == 2 || conn == 4 || conn == 6)
            {
               if (rad == 1)
               {
                  valence = conn;
                  hyd = 0;
               }
               else
                  hyd = -1; // will throw an error in the end
            }
            else if (conn > 7)
               hyd = -1; // will throw an error in the end
         }
      }
   }
   
   if (hyd < 0)
   {
      if (to_throw)
         throw Error("bad valence on %s having %d drawn bonds, charge %d, and %d radical electrons", toString(elem), conn, charge, rad);
      valence = conn;
      hyd = 0;
      return false;
   }
   return true;
}

int Element::calcValenceMinusHyd (int elem, int charge, int radical, int conn)
{
   int groupno = Element::group(elem);
   int rad = radicalElectrons(radical);

   if (groupno == 3)
   {
      if (elem == ELEM_B || elem == ELEM_Al || elem == ELEM_Ga || elem == ELEM_In)
      {
         if (charge == -1)
            if (rad + conn <= 4)
               return rad + conn;
      }
   }
   else if (groupno == 5)
   {
      if (elem == ELEM_N || elem == ELEM_P)
      {
         if (charge == 1)
            return rad + conn;
         if (charge == 2)
            return rad + conn;
      }
      else if (elem == ELEM_Sb || elem == ELEM_Bi || elem == ELEM_As)
      {
         if (charge == 1)
            return rad + conn;
         else if (charge == 2)
            return rad + conn;
      }
   }
   else if (groupno == 6)
   {
      if (elem == ELEM_O)
      {
         if (charge >= 1)
            return rad + conn;
      }
      else if (elem == ELEM_S  || elem == ELEM_Se || elem == ELEM_Po)
      {
         if (charge == 1 || charge == -1)
            return rad + conn;
      }
   }
   else if (groupno == 7)
   {
      if (elem == ELEM_Cl || elem == ELEM_Br ||
          elem == ELEM_I  || elem == ELEM_At)
      {
         if (charge == 1)
            return rad + conn;
      }
   }

   return rad + conn + abs(charge);
}

int Element::group (int elem)
{
   return _instance._element_parameters[elem].group;
}

int Element::period (int elem)
{
   return _instance._element_parameters[elem].period;
}

int Element::read (Scanner &scanner)
{
   char str[3] = {0, 0, 0};

   str[0] = scanner.readChar();

   if (islower(scanner.lookNext()))
      str[1] = scanner.readChar();

   return fromString(str);
}

void Element::_setStandardAtomicWeightIndex (int element, int index)
{
   _Parameters &p = _instance._element_parameters[element];
   p.natural_isotope_index = index;
}

void Element::_addElementIsotope (int element, int isotope, 
                                  float mass, float isotopic_composition)
{
   _instance._isotope_parameters_map.insert(
      _IsotopeKey(element, isotope), 
      _IsotopeValue(mass, isotopic_composition));
}

void Element::_initAllIsotopes ()
{
   #define ADD _addElementIsotope
   #define SET _setStandardAtomicWeightIndex
   #define NATURAL _IsotopeKey::NATURAL

   #include "elements_isotopes.inc"

   #undef ADD
   #undef SET
   #undef NATURAL

   _initDefaultIsotopes();
}

float Element::getStandardAtomicWeight (int element)
{
   _Parameters &p = _instance._element_parameters[element];
   return getRelativeIsotopicMass(element, p.natural_isotope_index);
}

int Element::getDefaultIsotope (int element)
{
   _Parameters &p = _instance._element_parameters[element];
   return p.default_isotope;
}

float Element::getIsotopicComposition (int element, int isotope)
{
   _IsotopeValue *value = _instance._isotope_parameters_map.at2(
      _IsotopeKey(element, isotope));

   if (value == 0)
      throw Error("getIsotopicComposition: isotope (%s, %d) not found",
         toString(element), isotope);

   return value->isotopic_composition;
}

void Element::getMinMaxIsotopeIndex  (int element, int &min, int &max)
{
   _Parameters &p = _instance._element_parameters[element];
   min = p.min_isotope_index;
   max = p.max_isotope_index;
}

float Element::getRelativeIsotopicMass (int element, int isotope)
{
   _IsotopeValue *value = _instance._isotope_parameters_map.at2(
      _IsotopeKey(element, isotope));

   if (value == 0)
      throw Error("getRelativeAtomicMass: isotope (%s, %d) not found",
         toString(element), isotope);

   return value->mass;
}

void Element::_initDefaultIsotopes ()
{
   Array<int> def_isotope_index;
   def_isotope_index.resize(_element_parameters.size());
   def_isotope_index.fffill();

   for (int i = ELEM_MIN; i < _element_parameters.size(); i++)
   {
      _element_parameters[i].default_isotope = -1;
      _element_parameters[i].min_isotope_index = 10000;
      _element_parameters[i].max_isotope_index = 0;
   }

   for (int i = _isotope_parameters_map.begin();
      i != _isotope_parameters_map.end(); 
      i = _isotope_parameters_map.next(i))
   {
      _IsotopeKey &key = _isotope_parameters_map.key(i);
      _IsotopeValue &value = _isotope_parameters_map.value(i);

      if (key.isotope == _IsotopeKey::NATURAL)
         continue;

      float atomic_weight = getStandardAtomicWeight(key.element);

      float diff_best = 1e6;
      if (def_isotope_index[key.element] != -1)
      {
         int best_iso = def_isotope_index[key.element];
         _IsotopeValue &best = _isotope_parameters_map.value(best_iso);

         diff_best = fabs(best.mass - atomic_weight);
      }
      float diff_cur = fabs(value.mass - atomic_weight);

      if (diff_best > diff_cur)
      {
         def_isotope_index[key.element] = i;
         _element_parameters[key.element].default_isotope = key.isotope;
      }

      int &min_iso = _element_parameters[key.element].min_isotope_index;
      int &max_iso = _element_parameters[key.element].max_isotope_index;
      if (min_iso > key.isotope)
         min_iso = key.isotope;
      if (max_iso < key.isotope)
         max_iso = key.isotope;
   }

   for (int i = ELEM_MIN; i < _element_parameters.size(); i++)
   {
      _Parameters &element = _element_parameters[i];

      if (element.natural_isotope_index != _IsotopeKey::NATURAL)
         element.default_isotope = element.natural_isotope_index;
   }

   // Post-condition
   for (int i = ELEM_MIN; i < _element_parameters.size(); i++)
      if (_element_parameters[i].default_isotope == -1)
         // usually you can't catch this as it's being thrown before main()
         throw Error("default isotope is not set on element #%d", i);
}

int Element::orbitals (int elem, bool use_d_orbital)
{
   int group = Element::group(elem);
   int period = Element::period(elem);
   switch (group)
   {
   case 1:
      return 1;
   case 2:
      return 2;
   default: 
      if (use_d_orbital && period > 2 && group >= 4)
         return 9;
      else
         return 4;
   }
}

int Element::electrons (int elem, int charge)
{
   return Element::group(elem) - charge;
}

int Element::getMaximumConnectivity (int elem, int charge, 
                                     int radical, bool use_d_orbital)
{
   int electrons = Element::electrons(elem, charge);
   int vacant_orbitals = Element::orbitals(elem, use_d_orbital) - radical;
   if (electrons <= vacant_orbitals)
      return electrons;
   else 
      return 2 * vacant_orbitals - electrons;
}

Element::_IsotopeKey::_IsotopeKey (int element, int isotope) :
   element(element), isotope(isotope)
{
}

bool Element::_IsotopeKey::operator< (const _IsotopeKey &right) const
{
   if (element < right.element)
      return true;
   if (element > right.element)
      return false;
   if (isotope < right.isotope)
      return true;
   if (isotope > right.isotope)
      return false;
   return false;
}

Element::_IsotopeValue::_IsotopeValue (float mass, float isotopic_composition) :
   mass(mass), isotopic_composition(isotopic_composition)
{
}

bool Element::canBeAromatic (int element)
{
   return _instance._element_parameters[element].can_be_aromatic;
}

void Element::_initAromatic ()
{
   int i;

   for (i = ELEM_B; i <= ELEM_F; i++)
      _element_parameters[i].can_be_aromatic = true;
   for (i = ELEM_Al; i <= ELEM_Cl; i++)
      _element_parameters[i].can_be_aromatic = true;
   for (i = ELEM_Ga; i <= ELEM_Br; i++)
      _element_parameters[i].can_be_aromatic = true;
   for (i = ELEM_In; i <= ELEM_I; i++)
      _element_parameters[i].can_be_aromatic = true;
   for (i = ELEM_Tl; i <= ELEM_Bi; i++)
      _element_parameters[i].can_be_aromatic = true;
}

Array<int> & Element::tautomerHeteroatoms ()
{
   return _instance._tau_heteroatoms;
}