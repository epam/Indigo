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

#ifndef __indigo_renderer_internal__
#define __indigo_renderer_internal__

#include "base_cpp/tlscont.h"
#include "indigo_internal.h"
#include "render_params.h"

class IndigoRenderer : public IndigoPluginContext
{
public:
    IndigoRenderer();
    ~IndigoRenderer();

    RenderParams renderParams;

    void init() override;

private:
    void setOptionsHandlers();

    bool options_set = false;
};

class IndigoHDCOutput : public IndigoObject
{
public:
    enum
    {
        HDC_OUTPUT = 110
    };

    IndigoHDCOutput(void* hdc, bool printing) : IndigoObject(HDC_OUTPUT), dc(hdc), prn(printing)
    {
    }
    void* dc;
    bool prn;

    ~IndigoHDCOutput() override
    {
    }

protected:
    RenderParams params;
};

// TL_DECL_EXT(IndigoRenderer, indigo_renderer_self);

#endif
