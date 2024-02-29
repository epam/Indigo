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

#include "indigo.h"

#define WRAPPER_LOAD_FROM_STRING(name)                                                                                                                         \
    CEXPORT int name##FromString(const char* string)                                                                                                           \
    {                                                                                                                                                          \
        int source = indigoReadString(string);                                                                                                                 \
        int result;                                                                                                                                            \
                                                                                                                                                               \
        if (source <= 0)                                                                                                                                       \
            return -1;                                                                                                                                         \
                                                                                                                                                               \
        result = name(source);                                                                                                                                 \
        indigoFree(source);                                                                                                                                    \
        return result;                                                                                                                                         \
    }

#define WRAPPER_LOAD_FROM_FILE(name)                                                                                                                           \
    CEXPORT int name##FromFile(const char* filename)                                                                                                           \
    {                                                                                                                                                          \
        int source = indigoReadFile(filename);                                                                                                                 \
        int result;                                                                                                                                            \
                                                                                                                                                               \
        if (source < 0)                                                                                                                                        \
            return -1;                                                                                                                                         \
                                                                                                                                                               \
        result = name(source);                                                                                                                                 \
        indigoFree(source);                                                                                                                                    \
        return result;                                                                                                                                         \
    }

#define WRAPPER_LOAD_FROM_BUFFER(name)                                                                                                                         \
    CEXPORT int name##FromBuffer(const char* buf, int size)                                                                                                    \
    {                                                                                                                                                          \
        int source = indigoReadBuffer(buf, size);                                                                                                              \
        int result;                                                                                                                                            \
                                                                                                                                                               \
        if (source < 0)                                                                                                                                        \
            return -1;                                                                                                                                         \
                                                                                                                                                               \
        result = name(source);                                                                                                                                 \
        indigoFree(source);                                                                                                                                    \
        return result;                                                                                                                                         \
    }

WRAPPER_LOAD_FROM_STRING(indigoLoadMolecule)
WRAPPER_LOAD_FROM_FILE(indigoLoadMolecule)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadMolecule)

WRAPPER_LOAD_FROM_STRING(indigoLoadQueryMolecule)
WRAPPER_LOAD_FROM_FILE(indigoLoadQueryMolecule)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadQueryMolecule)

WRAPPER_LOAD_FROM_STRING(indigoLoadSmarts)
WRAPPER_LOAD_FROM_FILE(indigoLoadSmarts)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadSmarts)

WRAPPER_LOAD_FROM_STRING(indigoLoadReaction)
WRAPPER_LOAD_FROM_FILE(indigoLoadReaction)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadReaction)

WRAPPER_LOAD_FROM_STRING(indigoLoadQueryReaction)
WRAPPER_LOAD_FROM_FILE(indigoLoadQueryReaction)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadQueryReaction)

WRAPPER_LOAD_FROM_STRING(indigoLoadReactionSmarts)
WRAPPER_LOAD_FROM_FILE(indigoLoadReactionSmarts)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadReactionSmarts)

CEXPORT int indigoSaveMolfileToFile(int molecule, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveMolfile(molecule, f);

    indigoFree(f);
    return res;
}

CEXPORT int indigoSaveJsonToFile(int item, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveJson(item, f);

    indigoFree(f);
    return res;
}

CEXPORT int indigoSaveSequenceToFile(int item, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveSequence(item, f);

    indigoFree(f);
    return res;
}

CEXPORT int indigoSaveFASTAToFile(int item, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveFASTA(item, f);

    indigoFree(f);
    return res;
}

CEXPORT int indigoSaveCmlToFile(int molecule, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveCml(molecule, f);

    indigoFree(f);
    return res;
}

CEXPORT const char* indigoMolfile(int molecule)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveMolfile(molecule, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}

CEXPORT const char* indigoSequence(int molecule)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveSequence(molecule, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}

CEXPORT const char* indigoFASTA(int molecule)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveFASTA(molecule, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}

CEXPORT const char* indigoCdxBase64(int object)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveCdx(object, b) == -1)
        return 0;

    res = indigoToBase64String(b);
    indigoFree(b);
    return res;
}

CEXPORT const char* indigoCml(int molecule)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveCml(molecule, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}

CEXPORT int indigoSaveRxnfileToFile(int reaction, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveRxnfile(reaction, f);

    indigoFree(f);
    return res;
}

CEXPORT const char* indigoRxnfile(int molecule)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveRxnfile(molecule, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}

CEXPORT int indigoSaveCdxmlToFile(int item, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveCdxml(item, f);

    indigoFree(f);
    return res;
}

CEXPORT int indigoSaveCdxToFile(int item, const char* filename)
{
    int f = indigoWriteFile(filename);
    int res;

    if (f == -1)
        return -1;

    res = indigoSaveCdx(item, f);

    indigoFree(f);
    return res;
}

CEXPORT const char* indigoCdxml(int item)
{
    int b = indigoWriteBuffer();
    const char* res;

    if (b == -1)
        return 0;

    if (indigoSaveCdxml(item, b) == -1)
        return 0;

    res = indigoToString(b);
    indigoFree(b);
    return res;
}
