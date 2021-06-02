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

#include "molecule/sdf_loader.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"

using namespace indigo;

IMPL_ERROR(SdfLoader, "SDF loader");

CP_DEF(SdfLoader);

SdfLoader::SdfLoader(Scanner& scanner) : CP_INIT, TL_CP_GET(data), TL_CP_GET(properties), TL_CP_GET(_offsets), TL_CP_GET(_preread)
{
    data.clear();
    properties.clear();

    // detect if input is gzipped
    byte id[2];
    long long pos = scanner.tell();

    scanner.readCharsFix(2, (char*)id);
    scanner.seek(pos, SEEK_SET);

    if (id[0] == 0x1f && id[1] == 0x8b)
    {
        _scanner = new GZipScanner(scanner);
        _own_scanner = true;
    }
    else
    {
        _scanner = &scanner;
        _own_scanner = false;
    }
    _current_number = 0;
    _max_offset = 0LL;
    _offsets.clear();
    _preread.clear();
}

SdfLoader::~SdfLoader()
{
    if (_own_scanner)
        delete _scanner;
}

long long SdfLoader::tell()
{
    return _scanner->tell();
}

int SdfLoader::currentNumber()
{
    return _current_number;
}

int SdfLoader::count()
{
    long long offset = _scanner->tell();
    int cn = _current_number;

    if (offset != _max_offset)
    {
        _scanner->seek(_max_offset, SEEK_SET);
        _preread.clear();
        _current_number = _offsets.size();
    }

    while (!isEOF())
        readNext();

    int res = _current_number;

    if (res != cn)
    {
        _scanner->seek(offset, SEEK_SET);
        _preread.clear();
        _current_number = cn;
    }

    return res;
}

bool SdfLoader::isEOF()
{
    // read space characters
    while (!_scanner->isEOF())
    {
        if (isspace(_scanner->lookNext()))
            _preread += _scanner->readChar();
        else
            return false;
    }

    // We are at the end of file now, having only space characters read.
    return true;
}

void SdfLoader::readNext()
{
    StringOutput output(data);
    output.writeArray(_preread);
    int n_preread = _preread.size();
    _preread.clear();
    QS_DEF(std::string, str);

    if (_scanner->isEOF())
        throw Error("end of stream");

    _offsets.expand(_current_number + 1);
    _offsets[_current_number++] = _scanner->tell() - n_preread;

    properties.clear();

    bool pending_emptyline = false;

    long long last_offset = -1LL;
    while (!_scanner->isEOF())
    {
        last_offset = _scanner->tell();
        _scanner->readLine(str);
        if (str.size() > 0 && str[0] == '>')
            break;
        if (str.size() > 3 && strncmp(str.c_str(), "$$$$", 4) == 0)
            break;
        if (pending_emptyline)
            output.printf("\n");
        pending_emptyline = str.empty();

        if (!pending_emptyline)
            output.writeStringCR(str.c_str());

        if (data.size() > MAX_DATA_SIZE)
            throw Error("data size exceeded the acceptable size %d bytes, Please check for correct file format", MAX_DATA_SIZE);
    }

    while (1)
    {
        if (strncmp(str.c_str(), "$$$$", 4) == 0)
            break;

        output.writeStringCR(str.c_str());

        BufferScanner ws(str.c_str());

        while (!ws.isEOF())
            if (ws.readChar() == '<')
                break;

        QS_DEF(std::string, word);
        bool have_word = false;

        word.clear();

        while (!ws.isEOF())
        {
            char c = ws.readChar();

            if (c == '>')
            {
                have_word = true;
                break;
            }
            word += c;
        }

        if (have_word && word.size() )
        {
            _scanner->readLine(str);
            auto& propBuf = properties.insert(word.c_str());
            //         auto& propBuf = properties.valueBuf(word.ptr());
            //         int idx = properties.findOrInsert(word.ptr());
            propBuf = str;
            output.writeStringCR(str.c_str());
            if (str.size() )
            {
                do
                {
                    if (_scanner->isEOF())
                        break;

                    _scanner->readLine(str);
                    output.writeStringCR(str.c_str());
                    if (str.size() )
                    {
                        propBuf += '\n';
                        propBuf += str;
                    }
                } while (str.size());
            }
        }

        if (_scanner->isEOF())
            break;

        _scanner->readLine(str);
    }

    if (_scanner->tell() > _max_offset)
        _max_offset = _scanner->tell();
}

void SdfLoader::readAt(int index)
{
    if (index < _offsets.size())
    {
        _scanner->seek(_offsets[index], SEEK_SET);
        _preread.clear();
        _current_number = index;
        readNext();
    }
    else
    {
        _scanner->seek(_max_offset, SEEK_SET);
        if (_scanner->isEOF())
        {
            throw Error("No such record index: %d", index);
        }

        _preread.clear();
        _current_number = _offsets.size();
        do
        {
            readNext();
        } while (index + 1 != _offsets.size());
    }
}
