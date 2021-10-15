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

#include "molecule/inchi_wrapper.h"

#include "indigo_internal.h"
#include "indigo_molecule.h"

//#define INDIGO_DEBUG

#ifdef INDIGO_DEBUG
#include <iostream>
#endif

using namespace indigo;

CEXPORT const char* indigoInchiVersion()
{
    return InchiWrapper::version();
}

//
// Session Inchi instance
//
class IndigoInchiContext : public IndigoPluginContext
{
public:
    InchiWrapper inchi;

    IndigoInchiContext()
    {
        indigo_id = TL_GET_SESSION_ID();
        setOptionsHandlers();
        init();
    }

    ~IndigoInchiContext()
    {
    }

    void init() override
    {
        inchi.clear();
    }

    void setOptionsHandlers();
};

static _SessionLocalContainer<IndigoInchiContext> inchi_wrapper_self;

IndigoInchiContext& indigoInchiGetInstance()
{
    IndigoInchiContext& inst = inchi_wrapper_self.getLocalCopy();
    inst.validate();
    return inst;
}

//
// C interface functions
//

CEXPORT int indigoInchiInit()
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "IndigoInchi(" << TL_GET_SESSION_ID() << ")";
    std::cout << ss.str() << std::endl;
#endif
    INDIGO_BEGIN_STATIC
    {
        IndigoInchiContext& inchi_context = inchi_wrapper_self.createOrGetLocalCopy();
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoInchiDispose()
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "~IndigoInchi(" << TL_GET_SESSION_ID() << ")";
    std::cout << ss.str() << std::endl;
#endif
    INDIGO_BEGIN_STATIC
    {
        inchi_wrapper_self.removeLocalCopy();
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoInchiResetOptions(void)
{
    IndigoInchiContext& inchi_context = indigoInchiGetInstance();
    inchi_context.init();
    return 0;
}

CEXPORT int indigoInchiLoadMolecule(const char* inchi_string)
{
    INDIGO_BEGIN
    {
        InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;

        std::unique_ptr<IndigoMolecule> mol_obj = std::make_unique<IndigoMolecule>();

        const char* aux_prefix = "AuxInfo";
        auto& tmp = self.getThreadTmpData();

        BufferScanner scanner(inchi_string);
        if (Scanner::isSingleLine(scanner))
        {
            scanner.readLine(tmp.string, true);

            if (strncmp(tmp.string.ptr(), aux_prefix, strlen(aux_prefix)) == 0)
                inchi_wrapper.loadMoleculeFromAux(tmp.string.ptr(), mol_obj->mol);
            else
                inchi_wrapper.loadMoleculeFromInchi(tmp.string.ptr(), mol_obj->mol);
        }
        else
        {
            while (!scanner.isEOF())
            {
                scanner.readLine(tmp.string, true);
                if (strncmp(tmp.string.ptr(), aux_prefix, strlen(aux_prefix)) == 0)
                {
                    inchi_wrapper.loadMoleculeFromAux(tmp.string.ptr(), mol_obj->mol);
                    break;
                }
                else
                    inchi_wrapper.loadMoleculeFromInchi(tmp.string.ptr(), mol_obj->mol);
            }
        }

        return self.addObject(mol_obj.release());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoInchiGetInchi(int molecule)
{
    INDIGO_BEGIN
    {
        InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
        IndigoObject& obj = self.getObject(molecule);

        auto& tmp = self.getThreadTmpData();
        inchi_wrapper.saveMoleculeIntoInchi(obj.getMolecule(), tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoInchiGetInchiKey(const char* inchi_string)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        InchiWrapper::InChIKey(inchi_string, tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoInchiGetWarning()
{
    InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
    if (inchi_wrapper.warning.size() != 0)
        return inchi_wrapper.warning.ptr();
    return "";
}

CEXPORT const char* indigoInchiGetLog()
{
    InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
    if (inchi_wrapper.log.size() != 0)
        return inchi_wrapper.log.ptr();
    return "";
}

CEXPORT const char* indigoInchiGetAuxInfo()
{
    InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
    if (inchi_wrapper.auxInfo.size() != 0)
        return inchi_wrapper.auxInfo.ptr();
    return "";
}

//
// Options
//

void indigoInchiSetInchiOptions(const char* options)
{
    InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
    inchi_wrapper.setOptions(options);
}

void indigoInchiGetInchiOptions(Array<char>& value)
{
    InchiWrapper& inchi_wrapper = indigoInchiGetInstance().inchi;
    inchi_wrapper.getOptions(value);
}

void IndigoInchiContext::setOptionsHandlers()
{
    auto mgr = sf::xlock_safe_ptr(indigoGetOptionManager(indigo_id));
    mgr->setOptionHandlerString("inchi-options", indigoInchiSetInchiOptions, indigoInchiGetInchiOptions);
}
