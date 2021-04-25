#include "bingo-nosql.h"
#include "indigo.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <thread>
#include <vector>

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"

#include "base_cpp/temporary_thread_obj.h"

#include "base_cpp/scanner.h"
#include "molecule/cmf_loader.h"
#include "molecule/cmf_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molfile_loader.h"
#include "molecule/sdf_loader.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

TEST(BingoNosqlTest, test_enumerate_id)
{

    int db = bingoCreateDatabaseFile("test.db", "molecule", "");
    int obj = indigoLoadMoleculeFromString("C1CCNCC1");
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);

    int count = 0;
    int e = bingoEnumerateId(db);
    while (bingoNext(e))
    {
        count++;
    }

    bingoEndSearch(e);

    ASSERT_EQ(count, 3);
}

TEST(BingoNosqlTest, test_loadtargetscmf)
{

    //   int db = bingoCreateDatabaseFile("test.db", "molecule", "");

    //   int sd = indigoIterateSDFile("../../bingo/data/molecules/resonance/import/targets/targets.sdf");

    FileScanner sc("../../bingo/data/molecules/resonance/import/targets/targets.sdf");

    SdfLoader sdf(sc);
    QueryMolecule qmol;

    Array<char> qbuf;
    qbuf.readString("N(#C)=C(C)C", false);
    BufferScanner sm_scanner(qbuf);
    SmilesLoader smiles_loader(sm_scanner);
    smiles_loader.loadQueryMolecule(qmol);

    //   while(!sdf.isEOF()) {
    //      sdf.readAt(137);
    //      printf("%s\n", sdf.data.ptr());
    //      sdf.readNext();
    sdf.readAt(138);
    //      printf("%d\n", sdf.currentNumber());
    try
    {
        BufferScanner bsc(sdf.data);
        MolfileLoader loader(bsc);
        //         loader.stereochemistry_options.ignore_errors = true;
        //         loader.ignore_bad_valence = true;
        Molecule mol;
        loader.loadMolecule(mol);
        //         printf("%d\n", mol.vertexCount());
        Array<char> buf;
        ArrayOutput buf_out(buf);
        CmfSaver cmf_saver(buf_out);

        cmf_saver.saveMolecule(mol);

        Molecule mol2;
        BufferScanner buf_in(buf);
        CmfLoader cmf_loader(buf_in);
        cmf_loader.loadMolecule(mol2);

        MoleculeSubstructureMatcher matcher(mol2);
        matcher.use_pi_systems_matcher = true;
        matcher.setQuery(qmol);
        matcher.find();
    }
    catch (Exception& e)
    {
        //         printf("%s\n", sdf.data.ptr());
        //         printf("%s", e.message());
        ASSERT_STREQ("", e.message());
    }
    //   }
    //   while(indigoHasNext(sd)) {
    //      int next_mol = indigoNext(sd);
    //      if(next_mol > 0) {
    //         printf("%s\n", indigoSmiles(next_mol));
    //      }
    //   }
    //   int obj = indigoLoadMoleculeFromString("C1CCNCC1");
    //   bingoInsertRecordObj(db, obj);
    //   bingoInsertRecordObj(db, obj);
    //   bingoInsertRecordObj(db, obj);
    //
    //   int count = 0;
    //   int e = bingoEnumerateId(db);
    //   while (bingoNext(e))
    //   {
    //       count++;
    //   }
    //
    //   bingoEndSearch(e);

    //   ASSERT_EQ(count, 3);
}