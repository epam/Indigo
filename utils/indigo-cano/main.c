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

//
// This is a command line utility for producing canonical SMILES
// or layered code for molecules in MOL or SDF format
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

void onError(const char* message, void* context)
{
    fflush(stdout);
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
    exit(-1);
}

void usage()
{
    printf("Usage:\n"
           "  indigo-cano filename.{mol,smi,cml,sdf,sdf.gz,rdf,rdf.gz} [parameters]\n"
           "  indigo-cano - SMILES [parameters]\n"
           "Parameters:\n"
           "  -smiles          Output canonical SMILES (default)\n"
           "  -layered         Output canonical layered code\n"
           "  -id <string>     ID field in SDF file\n"
           "  -no-arom         Do not aromatize molecules\n"
           "  -no-tetrahedral  Ignore tetrahedral stereocenters\n"
           "  -no-cistrans     Ignore cis-trans bonds information\n"
           "Examples:\n"
           "   indigo-cano infile.sdf\n"
           "   indigo-cano infile.sdf.gz -id molregno > results.txt\n"
           "   indigo-cano infile.smi -layered -no-cistrans\n"
           "   indigo-cano - 'NC1C=CC(O)=CC=1'\n");
}

int processMolecule(int mol, int smiles, int no_arom, int no_cistrans, int no_tetra)
{
    if (no_cistrans)
        if (indigoClearCisTrans(mol) < 0)
            return -1;

    if (no_tetra)
        if (!indigoClearStereocenters(mol))
            return -1;

    if (smiles && !no_arom)
        if (indigoAromatize(mol) < 0)
            return -1;

    if (smiles)
    {
        const char* res = indigoCanonicalSmiles(mol);
        if (res == 0)
            return -1;
        printf("%s\n", res);
    }
    else
    {
        const char* res = indigoLayeredCode(mol);
        if (res == 0)
            return -1;
        printf("%s\n", res);
    }
    return 1;
}

int main(int argc, char* argv[])
{
    int smiles = 1;
    int no_cistrans = 0;
    int no_tetra = 0;
    int no_arom = 0;
    int i = 2;
    const char* idfield = 0;
    const char* filename = 0;
    const char* ext = 0;

    if (argc < 2)
    {
        usage();
        return -1;
    }

    if (strcmp(argv[1], "-") != 0)
        filename = argv[1];
    else if (argc >= 3 && strcmp(argv[1], "-") == 0)
        i = 3;
    else
    {
        usage();
        return -1;
    }

    while (i < argc)
    {
        if (strcmp(argv[i], "-smiles") == 0)
            smiles = 1;
        else if (strcmp(argv[i], "-layered") == 0)
            smiles = 0;
        else if (strcmp(argv[i], "-no-cistrans") == 0)
            no_cistrans = 1;
        else if (strcmp(argv[i], "-no-arom") == 0)
            no_arom = 1;
        else if (strcmp(argv[i], "-no-tetrahedral") == 0)
            no_tetra = 1;
        else if (strcmp(argv[i], "-id") == 0)
        {
            if (++i >= argc)
            {
                fprintf(stderr, "expecting an identifier after -id\n");
                return -1;
            }

            idfield = argv[i];
        }
        else
        {
            fprintf(stderr, "unknown parameter: %s\n", argv[i]);
            return -1;
        }
        i++;
    }

    qword session = indigoAllocSessionId();
    indigoSetErrorHandler(onError, 0);

    if (filename == 0)
    {
        int mol = indigoLoadMoleculeFromString(argv[2]);
        processMolecule(mol, smiles, no_arom, no_cistrans, no_tetra);
        indigoFree(mol);
        indigoReleaseSessionId(session);
        return 0;
    }

    if (strlen(filename) > 4 && filename[strlen(filename) - 4] == '.')
        ext = filename + strlen(filename) - 3;
    else if (strlen(filename) > 7 && filename[strlen(filename) - 7] == '.')
        ext = filename + strlen(filename) - 6;
    else
    {
        fprintf(stderr, "input file format not recognized\n");
        indigoReleaseSessionId(session);
        return -1;
    }

    if (strcmp(ext, "mol") == 0)
    {
        int mol = indigoLoadMoleculeFromFile(filename);
        processMolecule(mol, smiles, no_arom, no_cistrans, no_tetra);
        indigoFree(mol);
        indigoReleaseSessionId(session);
        return 0;
    }
    else if (strcmp(ext, "cml") == 0 || strcmp(ext, "sdf") == 0 || strcmp(ext, "sdf.gz") == 0 || strcmp(ext, "rdf") == 0 || strcmp(ext, "rdf.gz") == 0 ||
             strcmp(ext, "smi") == 0 || strcmp(ext, "smi.gz") == 0)
    {
        int item, iter;

        if ((strstr(ext, "cml") != NULL))
            iter = indigoIterateCMLFile(filename);
        else if (strstr(ext, "sdf") != NULL)
            iter = indigoIterateSDFile(filename);
        else if (strstr(ext, "rdf") != NULL)
            iter = indigoIterateRDFile(filename);
        else
            iter = indigoIterateSmilesFile(filename);

        while ((item = indigoNext(iter)))
        {
            indigoSetErrorHandler(0, 0);
            if (processMolecule(item, smiles, no_arom, no_cistrans, no_tetra) == -1)
                printf("%s\n", indigoGetLastError());
            indigoSetErrorHandler(onError, 0);
            indigoFree(item);
        }
        indigoFree(iter);
    }
    else
    {
        fprintf(stderr, "input file format not recognized\n");
        indigoReleaseSessionId(session);
        return -1;
    }

    indigoReleaseSessionId(session);
    return 0;
}
