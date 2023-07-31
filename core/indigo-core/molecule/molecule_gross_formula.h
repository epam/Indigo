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

#ifndef __molecule_gross_formula__
#define __molecule_gross_formula__

#include <map>
#include <memory>
#include <set>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/red_black.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule_gross_formula_options.h"

namespace indigo
{

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

    class BaseMolecule;

    class DLLEXPORT GrossFormulaUnit
    {
    public:
        Array<char> multiplier;
        std::map<int, int> isotopes;
    };

    // Represents array of superunits gross formulas.
    typedef ObjArray<GrossFormulaUnit> GROSS_UNITS;

    class DLLEXPORT MoleculeGrossFormula
    {
    public:
        static void collect(BaseMolecule& molecule, Array<int>& gross);
        static std::unique_ptr<GROSS_UNITS> collect(BaseMolecule& molecule, bool add_isotopes = false);

        static void toString(const Array<int>& gross, Array<char>& str, bool add_rsites = false);
        static void toString(GROSS_UNITS& gross, Array<char>& str, bool add_rsites = false);
        static void toString_Hill(GROSS_UNITS& gross, Array<char>& str, bool add_rsites = false);
        static void fromString(const char* str, Array<int>& gross);
        static void fromString(Scanner& scanner, Array<int>& gross);

        static bool leq(const Array<int>& gross1, const Array<int>& gross2);
        static bool geq(const Array<int>& gross1, const Array<int>& gross2);
        static bool equal(const Array<int>& gross1, const Array<int>& gross2);

    protected:
        struct _ElemCounter
        {
            int elem;
            int isotope;
            int counter;
        };

        static void _toString(const Array<int>& gross, ArrayOutput& output, int (*cmp)(_ElemCounter&, _ElemCounter&, void*), bool add_rsites);
        static void _toString(const std::map<int, int>& gross, ArrayOutput& output, int (*cmp)(_ElemCounter&, _ElemCounter&, void*), bool add_rsites);
        static int _cmp(_ElemCounter& ec1, _ElemCounter& ec2, void* context);
        static int _cmp_hill(_ElemCounter& ec1, _ElemCounter& ec2, void* context);
        static int _cmp_hill_no_carbon(_ElemCounter& ec1, _ElemCounter& ec2, void* context);
        static int _isotopeCount(BaseMolecule& molecule);
    };

} // namespace indigo

#endif
