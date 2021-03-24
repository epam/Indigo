#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "indigo.h"

const char* issue338TestData = R"(<?xml version="1.0" ?>
<cml>
    <molecule title="">
        <atomArray>
            <atom id="a0" elementType="C" x2="8.95" y2="-8.525" />
            <atom id="a1" elementType="C" mrvPseudo="AH" x2="9.816" y2="-9.025" />
            <atom id="a2" elementType="C" x2="10.6821" y2="-8.525" />
        </atomArray>
        <bondArray>
            <bond atomRefs2="a0 a1" order="1" />
            <bond atomRefs2="a1 a2" order="1" />
        </bondArray>
    </molecule>
</cml>
)";

const size_t issue338TestLength = strlen(issue338TestData);

size_t mismatch(const char* s1, const char* s2)
{
    size_t i = 0;
    for (; s1[i] && s2[i]; ++i)
    {
        if (s1[i] != s2[i])
            break;
    }
    return i;
}

int main(void)
{
    int mol = indigoLoadMoleculeFromString(issue338TestData);
    if (mol == -1)
    {
        printf("indigoLoadMoleculeFromString failed: %s\n", indigoGetLastError());
        mol = indigoLoadQueryMoleculeFromString(issue338TestData);
    }
    if (mol == -1)
    {
        printf("indigoLoadQueryMoleculeFromString() failed: %s\n", indigoGetLastError());
        return 0;
    }

    int iter = indigoIterateAtoms(mol);
    while(indigoHasNext(iter))
    { 
        int atom = indigoNext(iter);
        const char* label = indigoSymbol(atom);
        bool is_pseudo = indigoIsPseudoatom(atom);
        int number = is_pseudo ? 0 : indigoAtomicNumber(atom);
        printf("atom: %s, number: %d, pseudo: %d\n", label, number, is_pseudo);
    }
    const char* str_save = indigoCml(mol);
    const size_t str_save_len = strlen(str_save);
    printf("%s\n", str_save);
    size_t pos = mismatch(issue338TestData, str_save);
    if (str_save_len == issue338TestLength && pos == issue338TestLength)
    {
        printf("OK\n");
    }
    else
    {
        printf("Error: output and input differ at position %zd.\n", pos);
    }
    indigoFree(mol);
    return 0;
}
