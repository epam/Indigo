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

#ifndef __molfile_commons__
#define __molfile_commons__

// Molfile keywords that the loader and the saver must spell alike.

namespace indigo
{
    // V2000: the tag of an S-group property line, "M  <tag>nn8 ...".
    const auto MOLFILE_SGROUP_TYPE_TAG = "STY";
    const auto MOLFILE_SGROUP_SUBTYPE_TAG = "SST";
    const auto MOLFILE_SGROUP_PARENT_TAG = "SPL";
    const auto MOLFILE_SGROUP_EXT_INDEX_TAG = "SLB";
    const auto MOLFILE_SGROUP_CONNECTIVITY_TAG = "SCN";
    const auto MOLFILE_SGROUP_COMPONENT_ORDER_TAG = "SNC";

    // V3000: the key of an S-group property, "<key>=<value>".
    const auto MOLFILE_SGROUP_SUBTYPE_KEY = "SUBTYPE";
    const auto MOLFILE_SGROUP_CONNECTIVITY_KEY = "CONNECT";
    const auto MOLFILE_SGROUP_COMPONENT_ORDER_KEY = "COMPNO";
} // namespace indigo

#endif
