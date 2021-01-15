#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"


extern const char* issue269test_RGroupsWarning;
extern const char* issue269test_RGroupsWarning_response;
int test_RGroupsWarning()
{
    int r = 0;
    const char * resp = indigoCheck2(issue269test_RGroupsWarning, "rgroup", "");
    printf("Test RGroupsWarning\n\tResponse: %s\n", resp);
    if (strcmp(resp, issue269test_RGroupsWarning_response))
    {
        printf("\nERROR: \n\tExpected:%s\n\tActual:%s\n\n", issue269test_RGroupsWarning_response, resp);
        r = 1;
    }
    return r;
}


const char* issue269test_ChiralityWarning;
const char* issue269test_StereochemistryWarning;
const char* issue269test_PseudoatomWarning;
const char* issue269test_RadicalWarning;
const char* issue269test_QueryWarning;

int main(void)
{
    int r = 0;
    r += test_RGroupsWarning();
    return r;
}
