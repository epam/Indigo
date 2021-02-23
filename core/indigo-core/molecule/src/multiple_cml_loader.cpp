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

#include "molecule/multiple_cml_loader.h"
#include "base_cpp/scanner.h"

using namespace indigo;

IMPL_ERROR(MultipleCmlLoader, "multiple CML loader");

CP_DEF(MultipleCmlLoader);

MultipleCmlLoader::MultipleCmlLoader(Scanner& scanner) : CP_INIT, TL_CP_GET(data), TL_CP_GET(_tags), TL_CP_GET(_offsets), _scanner(scanner)
{
    _tags.clear();
    _tags.push().readString("<reaction", false);
    _tags.push().readString("<molecule", false);
    _current_number = 0;
    _max_offset = 0LL;
    _offsets.clear();
    _reaction = false;
}

bool MultipleCmlLoader::isEOF()
{
    return _scanner.findWord(_tags) == -1;
}

void MultipleCmlLoader::readNext()
{
    int k = _scanner.findWord(_tags);

    if (k == -1)
        throw Error("end of stream");

    _offsets.expand(_current_number + 1);
    _offsets[_current_number++] = _scanner.tell();

    long long beg = _scanner.tell();
    long long size;

    if (k == 1)
    {
        if (!_scanner.findWord("</molecule>"))
            throw Error("no </molecule> tag");
        size = _scanner.tell() - beg + strlen("</molecule>");
        _reaction = false;
    }
    else
    {
        if (!_scanner.findWord("</reaction>"))
            throw Error("no </reaction> tag");
        size = _scanner.tell() - beg + strlen("</reaction>");
        _reaction = true;
    }

    _scanner.seek(beg, SEEK_SET);
    _scanner.read(static_cast<int>(size), data);

    if (_scanner.tell() > _max_offset)
        _max_offset = _scanner.tell();
}

long long MultipleCmlLoader::tell()
{
    return _scanner.tell();
}

int MultipleCmlLoader::currentNumber()
{
    return _current_number;
}

int MultipleCmlLoader::count()
{
    long long offset = _scanner.tell();
    int cn = _current_number;

    if (offset != _max_offset)
    {
        _scanner.seek(_max_offset, SEEK_SET);
        _current_number = _offsets.size();
    }

    while (!isEOF())
        readNext();

    int res = _current_number;

    if (res != cn)
    {
        _scanner.seek(offset, SEEK_SET);
        _current_number = cn;
    }

    return res;
}

void MultipleCmlLoader::readAt(int index)
{
    if (index < _offsets.size())
    {
        _scanner.seek(_offsets[index], SEEK_SET);
        _current_number = index;
        readNext();
    }
    else
    {
        _scanner.seek(_max_offset, SEEK_SET);
        if (_scanner.isEOF())
        {
            throw Error("No such record index: %d", index);
        }

        _current_number = _offsets.size();
        do
        {
            readNext();
        } while (index + 1 != _offsets.size());
    }
}

bool MultipleCmlLoader::isReaction()
{
    return _reaction;
}
