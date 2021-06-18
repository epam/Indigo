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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base_c/os_dir.h"
#include "indigo.h"

void onError(const char* message, void* context)
{
    fflush(stdout);
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
    exit(-1);
}

void _replaceSlashes(char* str)
{
    while (*str != 0)
    {
        if (*str == '\\')
            *str = '/';
        str++;
    }
}

void _handleInputFile(const char* path, int structures)
{
    if ((strlen(path) > 4 && strcmp(path + strlen(path) - 4, ".sdf") == 0) || (strlen(path) > 7 && strcmp(path + strlen(path) - 7, ".sdf.gz") == 0))
    {
        int item, iter = indigoIterateSDFile(path);

        while ((item = indigoNext(iter)))
        {
            indigoArrayAdd(structures, item);
            indigoFree(item);
        }
        indigoFree(iter);
    }
    else if ((strlen(path) > 4 && strcmp(path + strlen(path) - 4, ".rdf") == 0) || (strlen(path) > 7 && strcmp(path + strlen(path) - 7, ".rdf.gz") == 0))
    {
        int item, iter = indigoIterateRDFile(path);

        while ((item = indigoNext(iter)))
        {
            indigoArrayAdd(structures, item);
            indigoFree(item);
        }
        indigoFree(iter);
    }
    else if ((strlen(path) > 4 && strcmp(path + strlen(path) - 4, ".smi") == 0) || (strlen(path) > 7 && strcmp(path + strlen(path) - 7, ".smi.gz") == 0))
    {
        int item, iter = indigoIterateSmilesFile(path);

        while ((item = indigoNext(iter)))
        {
            indigoArrayAdd(structures, item);
            indigoFree(item);
        }
        indigoFree(iter);
    }
    else if ((strlen(path) > 4 && strcmp(path + strlen(path) - 4, ".cml") == 0))
    {
        int item, iter = indigoIterateCMLFile(path);

        while ((item = indigoNext(iter)))
        {
            indigoArrayAdd(structures, item);
            indigoFree(item);
        }
        indigoFree(iter);
    }
    else
    {
        int item = indigoLoadMoleculeFromFile(path);

        indigoArrayAdd(structures, item);
        indigoFree(item);
    }
}

void _printHelpMessage()
{
    printf("Usage:\n  indigo-deco files [options]\n"
           "Perfoms molecule scaffold detection and R-group deconvolution\n"
           "Accepted formats are: Molfile, SDFile, RDFile, SMILES, CML\n"
           "Options:\n"
           "-h          print this help message\n"
           "-a          calculate approximate scaffold (default is exact)\n"
           "-s  <file>  write maximum found scaffold to molfile\n"
           "-S  <file>  write all found scaffolds to SD-file\n"
           "-l  <file>  do not calculate scaffold, but load it from file\n"
           "-sr <file>  write scaffold with R-sites to a file\n"
           "-o  <file>  write resulting highlighted molecules to file\n"
           "-r  <file>  write resulting molecules with separated r-groups to file\n"
           "-na         no aromatic consideration\n"
           "--          marks end of options\n"
           "\nExamples:\n\n"
           "indigo-deco *.mol -o hl.sdf -s scaf.sdf\n"
           "  read molecules from molfiles in the current directory\n"
           "  save maximum found scaffold to scaf.mol\n"
           "  save highlighted molecules to hl.sdf\n"
           "indigo-deco structure.mol many.sdf -s scaf.mol -S allscafs.sdf -r rg.sdf \n"
           "  read one molecule from structure.mol and multiple molecules from many.sdf\n"
           "  save molecules with r-rgoups to rg.sdf\n"
           "  save all found scaffolds to allscafs.sdf\n"
           "indigo-deco *.smi -d readyscaf.mol -o hl.sdf\n"
           "  read multiple molecules from every SMILES file in the current directory\n"
           "  read scaffold from readyscaf.mol\n"
           "  save highlighted molecules to hl.sdf\n");
}

int main(int argc, const char** argv)
{
    int i;
    int done_with_options = 0;
    int approximate = 0;
    int scaffold = 0;
    int aromatic = 1;
    const char* outfile_hl = 0;
    const char* outfile_rg = 0;
    const char* outfile_maxscaf = 0;
    const char* outfile_allscafs = 0;
    const char* outfile_scaf_r = 0;
    int deco = 0;
    int structures = 0;

    const qword session = indigoAllocSessionId();
    indigoSetErrorHandler(onError, 0);

    printf("R-Group deconvolution utility, powered by Indigo API version %s\n", indigoVersion());

    structures = indigoCreateArray();

    indigoSetOptionBool("treat-x-as-pseudoatom", 1);
    indigoSetOptionBool("ignore-stereochemistry-errors", 1);

    for (i = 1; i < argc; i++)
    {
        if (!done_with_options && argv[i][0] == '-')
        {
            if (strcmp(argv[i], "--") == 0)
                done_with_options = 1;
            else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "-help") == 0 ||
                     strcmp(argv[i], "--help") == 0)
            {
                _printHelpMessage();
                return 0;
            }
            else if (strcmp(argv[i], "-a") == 0)
                approximate = 1;
            else if (strcmp(argv[i], "-l") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -l\n");
                    return -1;
                }
                scaffold = indigoLoadMoleculeFromFile(argv[i]);
            }
            else if (strcmp(argv[i], "-o") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -o\n");
                    return -1;
                }
                outfile_hl = argv[i];
            }
            else if (strcmp(argv[i], "-r") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -r\n");
                    return -1;
                }
                outfile_rg = argv[i];
            }
            else if (strcmp(argv[i], "-s") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -s\n");
                    return -1;
                }
                outfile_maxscaf = argv[i];
            }
            else if (strcmp(argv[i], "-sr") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -sr\n");
                    return -1;
                }
                outfile_scaf_r = argv[i];
            }
            else if (strcmp(argv[i], "-S") == 0)
            {
                if (++i == argc)
                {
                    fprintf(stderr, "expecting filename after -S\n");
                    return -1;
                }
                outfile_allscafs = argv[i];
            }
            else if (strcmp(argv[i], "-na") == 0)
                aromatic = 0;
            else
            {
                fprintf(stderr, "Unknown option: %s", argv[i]);
                _printHelpMessage();
                return -1;
            }
        }
        else
        {
            char dirname[1024];
            char errbuf[1024];
            const char* filename = 0;
            int k;

            for (k = (int)strlen(argv[i]) - 1; k >= 0; k--)
                if (argv[i][k] == '/' || argv[i][k] == '\\')
                    break;

            if (k == -1)
                strncpy(dirname, ".", sizeof(dirname));
            else if (k == 0)
            {
                dirname[0] = argv[i][0];
                dirname[1] = 0;
            }
            else if (k == strlen(argv[i]) - 1)
            {
                fprintf(stderr, "can not handle filenames ending with a slash\n");
                return -1;
            }
            else if (k > sizeof(dirname) - 1)
            {
                fprintf(stderr, "filename too long\n");
                return -1;
            }
            else
            {
                memcpy(dirname, argv[i], k);
                dirname[k] = 0;
            }

            _replaceSlashes(dirname);

            filename = argv[i] + k + 1;

            {
                OsDirIter dir_iter;
                int rc = osDirSearch(dirname, filename, &dir_iter);

                if (rc == OS_DIR_OK)
                {
                    int count = 0;

                    while ((rc = osDirNext(&dir_iter)) == OS_DIR_OK)
                    {
                        _replaceSlashes(dir_iter.path);
                        _handleInputFile(dir_iter.path, structures);
                        count++;
                    }
                    if (rc != OS_DIR_END)
                    {
                        fprintf(stderr, "%s\n", osDirLastError(errbuf, sizeof(errbuf)));
                        return -1;
                    }
                    if (count == 0)
                    {
                        fprintf(stderr, "can not find %s in directory %s\n", filename, dirname);
                        return -1;
                    }
                }
                else
                {
                    fprintf(stderr, "%s\n", osDirLastError(errbuf, sizeof(errbuf)));
                    return -1;
                }
            }
        }
    }

    if (indigoCount(structures) < 1)
    {
        fprintf(stderr, "no input structures\n");
        _printHelpMessage();
        return -1;
    }

    printf("got %d input structures\n", indigoCount(structures));

    indigoSetOptionBool("deconvolution-aromatization", aromatic);

    if (scaffold == 0)
    {
        printf("calculating scaffold... ");
        fflush(stdout);
        if (approximate)
            scaffold = indigoExtractCommonScaffold(structures, "approximate");
        else
            scaffold = indigoExtractCommonScaffold(structures, "exact");
        printf("done\n");
        fflush(stdout);
    }

    if (outfile_maxscaf != 0)
    {
        printf("saving the scaffold to %s\n", outfile_maxscaf);
        indigoSaveMolfileToFile(scaffold, outfile_maxscaf);
    }

    if (outfile_allscafs != 0)
    {
        int output = indigoWriteFile(outfile_allscafs);
        int allscafs = indigoAllScaffolds(scaffold);
        int item, iter = indigoIterateArray(allscafs);

        printf("saving all obtained scaffolds (%d total) to %s\n", indigoCount(allscafs), outfile_allscafs);

        while ((item = indigoNext(iter)))
        {
            indigoSdfAppend(output, item);
            indigoFree(item);
        }
        indigoFree(iter);
        indigoFree(output);
    }

    if (outfile_hl == 0 && outfile_rg == 0 && outfile_scaf_r == 0)
    {
        printf("none of -o, -r, -sr specified, nothing left to do\n");
        return 0;
    }

    printf("decomposing the structures... ");
    fflush(stdout);
    deco = indigoDecomposeMolecules(scaffold, structures);
    printf("done\n");
    fflush(stdout);

    if (outfile_scaf_r != 0)
    {
        int sr = indigoDecomposedMoleculeScaffold(deco);
        indigoLayout(sr);
        printf("saving the scaffold with R-sites to %s\n", outfile_scaf_r);
        indigoSaveMolfileToFile(sr, outfile_scaf_r);
    }

    if (outfile_hl != 0)
    {
        int output = indigoWriteFile(outfile_hl);
        int item, iter = indigoIterateDecomposedMolecules(deco);

        printf("saving the highlighted structures to %s\n", outfile_hl);

        while ((item = indigoNext(iter)))
        {
            indigoSdfAppend(output, indigoDecomposedMoleculeHighlighted(item));
            indigoFree(item);
        }

        indigoFree(iter);
        indigoFree(output);
    }

    if (outfile_rg != 0)
    {
        int output = indigoWriteFile(outfile_rg);
        int item, iter = indigoIterateDecomposedMolecules(deco);

        printf("saving the structures with R-groups to %s\n", outfile_rg);

        while ((item = indigoNext(iter)))
        {
            indigoSdfAppend(output, indigoDecomposedMoleculeWithRGroups(item));
            indigoFree(item);
        }

        indigoFree(iter);
        indigoFree(output);
    }

    indigoReleaseSessionId(session);
    return 0;
};
