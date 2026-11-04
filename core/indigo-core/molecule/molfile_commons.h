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
    const auto MOLFILE_SGROUP_QUERY_TYPE_KEY = "QUERYTYPE";
    const auto MOLFILE_SGROUP_QUERY_OPERATOR_KEY = "QUERYOP";
    const auto MOLFILE_SGROUP_FIELD_DISPLAY_KEY = "FIELDDISP";

    // M  SDD and FIELDDISP: the number of characters to display when it is every character
    const auto MOLFILE_DATA_ALL_CHARACTERS = "ALL";

    // M  SDT: the widths of its columns after the S-group number - field name, field type, units or format, query
    // type, query operator; the reader takes each by its width, the writer pads each to it
    constexpr int MOLFILE_DATA_FIELD_NAME_WIDTH = 30;
    constexpr int MOLFILE_DATA_FIELD_TYPE_WIDTH = 2;
    constexpr int MOLFILE_DATA_FIELD_UNITS_WIDTH = 20;
    constexpr int MOLFILE_DATA_QUERY_TYPE_WIDTH = 2;
    constexpr int MOLFILE_DATA_QUERY_OPERATOR_WIDTH = 15;
    // M  SCD and M  SED: a line of the data is cut to this many characters when read, and is not written in V2000
    // when longer
    constexpr int MOLFILE_V2000_MAX_DATA_LINE_LENGTH = 200;
    // M  STY and the other property lines number an S-group in three columns
    constexpr int MOLFILE_V2000_MAX_SGROUP_COUNT = 999;
    // F10.4, the column of a coordinate in V2000 and of the position of a data value in both versions: ten columns,
    // four of them decimals, one a sign
    constexpr int MOLFILE_F10_4_WIDTH = 10;
    constexpr double MOLFILE_F10_4_MIN = -9999.9999;
    constexpr double MOLFILE_F10_4_MAX = 99999.9999;
    inline bool molfileFitsF10_4(double value)
    {
        return value >= MOLFILE_F10_4_MIN && value <= MOLFILE_F10_4_MAX;
    }
} // namespace indigo

#endif
