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

#include <memory>
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "indigo_internal.h"

CEXPORT int indigoNext(int iter)
{
    INDIGO_BEGIN
    {
        IndigoObject* nextobj = self.getObject(iter).next();

        if (nextobj == 0)
            return 0;

        return self.addObject(nextobj);
    }
    INDIGO_END(-1);
}

CEXPORT int indigoHasNext(int iter)
{
    INDIGO_BEGIN
    {
        return self.getObject(iter).hasNext() ? 1 : 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIndex(int handle)
{
    INDIGO_BEGIN
    {
        return self.getObject(handle).getIndex();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClone(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        return self.addObject(obj.clone());
    }
    INDIGO_END(-1);
}
