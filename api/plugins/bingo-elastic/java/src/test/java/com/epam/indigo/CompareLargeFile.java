package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatch;
import org.junit.Ignore;
import org.junit.jupiter.api.*;

import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

@Ignore
public class CompareLargeFile extends NoSQLElasticCompareAbstract {

    protected static String test100SmilesFile = "src/test/resources/pubchem_slice_100000.smiles";
    protected static String smiles[] = new String[]{
            "CC(=C)C(=O)NC1C=CC=CC=1C([O-])=O",
            "NC(C=CC=CCCCCCCCC)C"
    };

    private static void loadFile()  {
        long noSQLTotal = System.nanoTime();
        for (IndigoObject indigoObject : indigo.iterateSmilesFile(test100SmilesFile)) {
            bingoDb.insert(indigoObject);
        }
        noSQLTotal = System.nanoTime() - noSQLTotal;
        long elasticTotal = System.nanoTime();
        try {
            List<IndigoRecord> indigoRecordList =
                    Helpers.loadFromSmilesFile(test100SmilesFile, false);
            repository.indexRecords(indigoRecordList);
        } catch (Exception e) {
            Assertions.fail(e);
        }
        elasticTotal = System.nanoTime() - elasticTotal;
        assertTrue(elasticTotal < noSQLTotal);

        try {
            TimeUnit.SECONDS.sleep(1);
        } catch (InterruptedException e) {
            Assertions.fail(e);
        }
    }

    @BeforeAll
    public static void setUp() {
        setUpDataStore();
        loadFile();
    }

    @AfterAll
    public static void tearDown() {
        tearDownDataStore();
    }

    @Test
    @DisplayName("Tanimoto test")
    public void tanimoto() {

    }

    @Test
    @DisplayName("Euclid test")
    public void euclid() {

    }

    @Test
    @DisplayName("Tversky test")
    public void tversky() {

    }

    @Test
    @DisplayName("Exact match")
    public void exactMatch() {
        long noSQLTotal = System.nanoTime();
        IndigoRecord elasticNeedle = Helpers.loadFromSmiles(smiles[1]);
        IndigoObject bingoNeedle = indigo.loadMolecule(smiles[1]);
        BingoObject bingoObjectResult = bingoDb.searchExact(bingoNeedle);
        noSQLTotal = System.nanoTime() - noSQLTotal;
        assertTrue(bingoObjectResult.next());

        long elasticTotal = System.nanoTime();
        // TODO: no results found
        List<IndigoRecord> indigoResult = repository.stream().limit(1).filter(
                new ExactMatch<>(elasticNeedle)).collect(Collectors.toList());
        elasticTotal = System.nanoTime() - elasticTotal;

        assertTrue(elasticTotal < noSQLTotal);

        // assertEquals(smiles[1], indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
    }
}
