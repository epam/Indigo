/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __elements_h__
#define __elements_h__

#include <array>
#include <map>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;

    enum
    {
        ELEM_MIN = 1,
        ELEM_H = 1,
        ELEM_He,
        ELEM_Li,
        ELEM_Be,
        ELEM_B,
        ELEM_C,
        ELEM_N,
        ELEM_O,
        ELEM_F,
        ELEM_Ne,
        ELEM_Na,
        ELEM_Mg,
        ELEM_Al,
        ELEM_Si,
        ELEM_P,
        ELEM_S,
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
        ELEM_Db,
        ELEM_Sg,
        ELEM_Bh,
        ELEM_Hs,
        ELEM_Mt,
        ELEM_Ds,
        ELEM_Rg,
        ELEM_Cn,
        ELEM_Nh, // ELEM_Uut
        ELEM_Fl,
        ELEM_Mc, // ELEM_Uup
        ELEM_Lv,
        ELEM_Ts, // ELEM_Uus
        ELEM_Og, // ELEM_Uuo
        ELEM_MAX,
        ELEM_PSEUDO,   // pseudoatom
        ELEM_RSITE,    // 'R' atom
        ELEM_TEMPLATE, // template
        ELEM_ATTPOINT, // attachment point
        ELEM_ATOMLIST
    };

    enum
    {
        RADICAL_SINGLET = 1,
        RADICAL_DOUBLET = 2,
        RADICAL_TRIPLET = 3
    };

    enum
    {
        HYDROGEN = 1,
        DEUTERIUM = 2,
        TRITIUM = 3
    };

    class DLLEXPORT Element
    {
    public:
        DECL_ERROR;

        static const char* toString(int element);
        static int fromString(const char* name);
        static int fromString2(const char* name);
        static int fromChar(char c);
        static int fromTwoChars(char c1, char c2);
        static int fromTwoChars2(char c1, char c2);

        static int radicalElectrons(int radical);
        static int radicalOrbitals(int radical);

        static bool calcValence(int elem, int charge, int radical, int conn, int& valence, int& hyd, bool to_throw);
        static int calcValenceOfAromaticAtom(int elem, int charge, int n_arom, int min_conn);
        static int calcValenceMinusHyd(int elem, int charge, int radical, int conn);

        // Calculate maximum number of single bonds that
        // can be attached to specified atom.
        static int getMaximumConnectivity(int elem, int charge, int radical, bool use_d_orbital);
        static int orbitals(int elem, bool use_d_orbital);
        static int electrons(int elem, int charge);

        static int group(int element);
        static int period(int period);
        static int read(Scanner& scanner);

        static bool isHalogen(int element);

        // Returns isotope that has weight most close to the atomic weight
        static int getDefaultIsotope(int element);
        // Return most abundant isotope
        static int getMostAbundantIsotope(int element);
        static double getRelativeIsotopicMass(int element, int isotope);
        static double getStandardAtomicWeight(int element);
        static bool getIsotopicComposition(int element, int isotope, double& res);
        static void getMinMaxIsotopeIndex(int element, int& min, int& max);

        static bool canBeAromatic(int element);

        static int getNumOuterElectrons(int element);

        Element(const Element&) = delete;
        Element(Element&&) = delete;
        Element& operator=(const Element&) = delete;
        Element& operator=(Element&&) = delete;
        static const Element& _instance();

    private:
        Element();
        ~Element() = default;

        void _initAllPeriodic();
        void _initPeriodic(int element, const char* name, int period, int group);

        void _setStandardAtomicWeightIndex(int element, int index);
        void _addElementIsotope(int element, int isotope, double mass, double isotopic_composition);
        void _initAllIsotopes();
        void _initDefaultIsotopes();
        void _initAromatic();

        double _getStandardAtomicWeight(int element) const;
        double _getRelativeIsotopicMass(int element, int isotope) const;

        // Per-element physical parameters
        struct ElementParameters
        {
            char name[3];

            int group;
            int period;

            int natural_isotope_index; // Can be ElementIsotope::NATURAL or anything else
            // Isotope with mass most close to the atomic weight
            int default_isotope;
            // Isotope with highest mole fraction
            int most_abundant_isotope;
            // Minimum and maximum isotope index
            int min_isotope_index, max_isotope_index;

            bool can_be_aromatic;
        };

        // Isotopes mass key
        struct DLLEXPORT IsotopeKey
        {
            int element;
            int isotope; // Can be equal to NATURAL

            // Pseudoisotope value for weighted sum of the
            // atomic masses of all the isotopes for specified element
            enum
            {
                NATURAL = -1,
                UNINITIALIZED = -2
            };

            bool operator<(const IsotopeKey& right) const;
        };

        struct IsotopeValue
        {
            // Isotope mass
            double mass;
            // Mole fraction of the various isotopes
            double isotopic_composition;
        };

        struct cmp_str
        {
            bool operator()(char const* a, char const* b) const
            {
                return std::strcmp(a, b) < 0;
            }
        };

        std::map<const char*, int, cmp_str> _map;
        std::array<ElementParameters, ELEM_MAX> _element_parameters;
        std::map<IsotopeKey, IsotopeValue> _isotope_parameters_map;
    };
} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
