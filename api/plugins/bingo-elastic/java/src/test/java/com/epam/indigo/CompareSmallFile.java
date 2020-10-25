package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.EuclidSimilarityMatch;
import com.epam.indigo.predicate.ExactMatch;
import com.epam.indigo.predicate.TanimotoSimilarityMatch;
import com.epam.indigo.predicate.TverskySimilarityMatch;
import org.junit.jupiter.api.*;

import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class CompareSmallFile extends NoSQLElasticCompareAbstract {

    protected static String testSdfFile = "src/test/resources/zinc-slice.sdf";

    protected static String smiles[] = new String[]{"CC(=C)C(=O)NC1C=CC=CC=1C([O-])=O"};

    protected static IndigoObject bingoNeedle;
    protected static IndigoRecord elasticNeedle;

    @BeforeAll
    public static void setUp() throws Exception {
        setUpDataStore();
        loadFile();
        bingoNeedle = indigo.loadMolecule(smiles[0]);
        elasticNeedle = Helpers.loadFromSmiles(smiles[0]);
    }


    protected static void loadFile() {
        for (IndigoObject indigoObject : indigo.iterateSDFile(testSdfFile)) {
            bingoDb.insert(indigoObject);
        }
        try {
            List<IndigoRecord> indigoRecordList =
                    Helpers.loadFromSdf(testSdfFile);
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @AfterAll
    public static void tearDown() {
        tearDownDataStore();
    }

    @Test
    @DisplayName("Tanimoto test")
    public void tanimoto() {

        // Tanimoto bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.9f, 1, "tanimoto");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertEquals(indigoObjectResult.canonicalSmiles(), "CC(=C)C(=O)Nc1ccccc1C([O-])=O");
        assertFalse(bingoObjectResult.next());

        // Tanimoto elastic
        List<IndigoRecord>  indigoResult = repository.stream().limit(10).filter(
                new TanimotoSimilarityMatch<>(elasticNeedle, 0.99f))
                .collect(Collectors.toList());
        assertEquals(smiles[0], indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
    }

    @Test
    @DisplayName("Euclid test")
    public void euclid() {

        // Euclid bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "euclid-sub");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertEquals(indigoObjectResult.canonicalSmiles(), "CC(=C)C(=O)Nc1ccccc1C([O-])=O");
        assertFalse(bingoObjectResult.next());

        // Euclid elastic
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                new EuclidSimilarityMatch<>(elasticNeedle, 0.95f))
                .collect(Collectors.toList());
        assertEquals(smiles[0], indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(6, indigoResult.size());
    }

    @Test
    @DisplayName("Tversky test")
    public void tversky() {

        // Tversky bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "euclid-sub");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertEquals(indigoObjectResult.canonicalSmiles(), "CC(=C)C(=O)Nc1ccccc1C([O-])=O");
        assertFalse(bingoObjectResult.next());

        // Tversky elastic
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                new TverskySimilarityMatch<>(elasticNeedle, 1, 1, 1))
                .collect(Collectors.toList());
        assertEquals(smiles[0], indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
    }

    @Test
    @DisplayName("Exact match")
    public void exactMatch() {

        List<IndigoRecord> indigoResult = repository.stream().limit(1).filter(
                new ExactMatch<>(elasticNeedle)).collect(Collectors.toList());

        assertEquals(smiles[0], indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
    }

}
