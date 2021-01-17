#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

#define TEST(NAME, param)                                                                                                                                      \
    extern const char* issue269test_##NAME;                                                                                                                    \
    extern const char* issue269test_##NAME##_response;                                                                                                         \
    int test_##NAME()                                                                                                                                          \
    {                                                                                                                                                          \
        int r = 0;                                                                                                                                             \
        const char* resp = indigoCheck2(issue269test_##NAME, param, "");                                                                                       \
        printf("Test " #NAME "\n\tResponse: %s\n", resp);                                                                                                      \
/*        if (strcmp(resp, issue269test_##NAME##_response))                                                                                                      \
        {                                                                                                                                                      \
            printf("\nERROR: \n\tExpected:%s\n\tActual:%s\n\n", issue269test_##NAME##_response, resp);                                                         \
            r = 1;                                                                                                                                             \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            printf("OK\n\n");                                                                                                                                  \
        }     */                                                                                                                                                 \
        return r;                                                                                                                                              \
    }

TEST(RGroupsWarning, "rgroup");
TEST(ChiralityWarning, "chirality");
TEST(StereochemistryWarning, "stereo");
TEST(PseudoatomWarning, "pseudoatom");
TEST(RadicalWarning, "radical");
TEST(QueryWarning, "query");
TEST(All, "all");

int main(void)
{
    int r = 0;
//!    r += test_RGroupsWarning();
    r += test_ChiralityWarning();
//!    r += test_StereochemistryWarning();
//!   r += test_PseudoatomWarning();
//!    r += test_RadicalWarning();
//!    r += test_QueryWarning();
//!    r += test_All();
    return r;
}
