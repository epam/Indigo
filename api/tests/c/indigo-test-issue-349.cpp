#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

const char* CID58374005TestData = R"(CID 58374005 edited
  -INDIGO-10082014492D

  5  4  0  0  0  0  0  0  0  0999 V2000
    9.3739   -2.8820    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    9.1588   -3.6784    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.7410   -4.2630    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.3615   -3.8904    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.5430   -3.5061    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  2  4  2  3  0  0  0
  4  5  1  0  0  0  0
M  RAD  1   4   2
M  END
)";

const char* issue349TestData1 = R"(-
MolEngine02082107562D

  4  3  0  0  0  0            999 V2000
    1.3520    1.5600    0.0000 C 
    2.7020    2.3400    0.0000 Br
    1.3520    0.0000    0.0000 C 
    0.0000    2.3400    0.0000 O 
  1  2  1
  1  3  1
  1  4  1
M  END
)";

const char* issue349TestData2 = R"(-
MolEngine02082107492D

  4  3  0  0  0  0            999 V2000
    1.3510    1.6040    0.0000 C 
    2.7020    2.3400    0.0000 C 
    1.3510    0.0000    0.0000 O 
    0.0000    2.3400    0.0000 N 
  1  2  1  4
  1  3  1
  1  4  1
M  END
)";

const char* issue349TestData5 = R"(-
MolEngine02082108432D

  5  4  0  0  0  0            999 V2000
    0.0000    0.0000    0.0000 C 
    1.3510    0.7800    0.0000 C   0  0  1
    1.3510    2.3400    0.0000 O 
    2.7010    0.0000    0.0000 C 
    4.0530    0.7800    0.0000 C 
  1  2  1
  2  3  1  1
  2  4  1
  4  5  1
M  END
)";

const char* issue349TestData1Trimmed = R"(-
MolEngine02082107562D

  4  3  0  0  0  0            999 V2000
    1.3520    1.5600    0.0000 C
    2.7020    2.3400    0.0000 Br
    1.3520    0.0000    0.0000 C
    0.0000    2.3400    0.0000 O
  1  2  1
  1  3  1
  1  4  1
M  END
)";


int main(void)
{
    printf("indigo-c-test-shared-issue-349\n");
    indigoSetOptionBool("ignore-stereochemistry-errors", 1);

    const char* testSamples[] = { CID58374005TestData, issue349TestData1, issue349TestData2, issue349TestData5, issue349TestData1Trimmed };
    
    int fail_count = 0;
    for (const auto testSample : testSamples)
    {
        int mol = indigoLoadMoleculeFromString(testSample);
        if (mol != -1)
        {
            indigoFree(mol);
            printf("OK\n");
        }
        else
        {
            ++fail_count;
            printf("Failed: %s\n", indigoGetLastError());
        }
    }

    return fail_count;
}
