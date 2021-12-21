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

#ifndef __molecule_inchi_h__
#define __molecule_inchi_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "molecule/molecule_inchi_component.h"
#include "molecule/molecule_inchi_layers.h"

namespace indigo
{

    class Output;
    class Graph;

    // Molecule InChI code constructor class
    class MoleculeInChI
    {
    public:
        explicit MoleculeInChI(Output& output);

        // InChI version. By default it is "Indigo=1.1"
        const char* prefix;

        // Save InChI code to the output
        void outputInChI(Molecule& mol);

        DECL_ERROR;

    private:
        //
        // Components compare methods
        //

        // Compare components. Returns DIFFERENCE_**** for the first found difference
        static int _cmpComponents(int& index1, int& index2, void* context);

        //
        // Printing
        //
        void _printInChI();

        class _PrintLayerFuncBase
        {
        public:
            virtual ~_PrintLayerFuncBase()
            {
            }

            virtual void operator()(MoleculeInChICompoment& comp, Array<char>& result) = 0;
        };
        template <typename Layer>
        class _ComponentLayerPrintFunction;

        bool _printInChILayer(_PrintLayerFuncBase& func, const char* delim, const char* multiplier, const char* layer_prefix);

        void _printInChIComponentCisTrans(MoleculeInChICompoment& comp, Array<char>& result);

        static void _normalizeMolecule(Molecule& mol);

        Output& _output;

        // Array with molecule components and InChI information and sorted indices
        CP_DECL;
        TL_CP_DECL(ReusableObjArray<MoleculeInChICompoment>, _components);
        TL_CP_DECL(Array<int>, _component_indices);
    };

} // namespace indigo

#endif // __molecule_inchi_h__
