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

#ifndef __indigo_cpp__
#define __indigo_cpp__

#include "indigo.h"

class IndigoAutoObj
{
public:
    IndigoAutoObj(int obj_id = -1)
    {
        _obj_id = obj_id;
    }

    ~IndigoAutoObj()
    {
        free();
    }

    void free()
    {
        if (_obj_id != -1)
        {
            indigoFree(_obj_id);
            _obj_id = -1;
        }
    }

    operator int() const
    {
        return _obj_id;
    }

    IndigoAutoObj& operator=(int id)
    {
        free();
        _obj_id = id;
        return *this;
    }

private:
    int _obj_id;
};

#endif // __indigo_cpp__
