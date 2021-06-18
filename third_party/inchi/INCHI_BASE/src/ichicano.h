/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */


#ifndef _INCHICANO_H_
#define _INCHICANO_H_


#include "ichicant.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


    int GetCanonLengths( int num_at,
                         sp_ATOM* at,
                         ATOM_SIZES *s,
                         T_GROUP_INFO *t_group_info );
    int AllocateCS( CANON_STAT *pCS,
                    int num_at,
                    int num_at_tg,
                    int nLenCT,
                    int nLenCTAtOnly,
                    int nLenLinearCTStereoDble,
                    int nLenLinearCTIsotopicStereoDble,
                    int nLenLinearCTStereoCarb,
                    int nLenLinearCTIsotopicStereoCarb,
                    int nLenLinearCTTautomer,
                    int nLenLinearCTIsotopicTautomer,
                    int nLenIsotopic,
                    INCHI_MODE nMode,
                    BCN *pBCN );
    int DeAllocateCS( CANON_STAT *pCS );
    void DeAllocBCN( BCN *pBCN );

    struct tagINCHI_CLOCK;

    int Canon_INChI( struct tagINCHI_CLOCK *ic,
                     int num_atoms,
                     int num_at_tg,
                     sp_ATOM* at,
                     CANON_STAT* pCS,
                     CANON_GLOBALS *pCG,
                     INCHI_MODE nMode,
                     int bTautFtcn );
    int GetBaseCanonRanking( struct tagINCHI_CLOCK *ic,
                             int num_atoms,
                             int num_at_tg,
                             sp_ATOM* at[],
                             T_GROUP_INFO *t_group_info,
                             ATOM_SIZES s[],
                             BCN *pBCN,
                             struct tagInchiTime *ulTimeOutTime,
                             CANON_GLOBALS *pCG,
                             int bFixIsoFixedH,
                             int LargeMolecules );
    int bCanonIsFinerThanEquitablePartition( int num_atoms,
                                             sp_ATOM* at,
                                             AT_RANK *nSymmRank );
    int UpdateFullLinearCT( int num_atoms,
                            int num_at_tg,
                            sp_ATOM* at,
                            AT_RANK *nRank,
                            AT_RANK *nAtomNumber,
                            CANON_STAT* pCS,
                            CANON_GLOBALS *pCG,
                            int bFirstTime );
    int FixCanonEquivalenceInfo( CANON_GLOBALS *pCG,
                                 int num_at_tg,
                                 AT_RANK *nSymmRank,
                                 AT_RANK *nCurrRank,
                                 AT_RANK *nTempRank,
                                 AT_NUMB *nAtomNumber,
                                 int *bChanged );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* _INCHICANO_H_ */
