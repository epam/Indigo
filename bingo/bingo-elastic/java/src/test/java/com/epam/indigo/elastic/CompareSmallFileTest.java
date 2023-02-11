package com.epam.indigo.elastic;

import com.epam.indigo.BingoObject;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.predicate.*;
import org.junit.jupiter.api.*;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;


public class CompareSmallFileTest extends NoSQLElasticCompareAbstract {

    protected static final String testSdfFile = "src/test/resources/zinc-slice.sdf.gz";

    protected static final String[] smiles = new String[]{"CC(=C)C(=O)NC1C=CC=CC=1C([O-])=O"};

    protected static IndigoObject bingoNeedle;
    protected static IndigoRecord elasticNeedle;

    @BeforeAll
    public static void setUp() {
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
            List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
            Helpers.iterateSdf(testSdfFile).forEach(indigoRecordList::add);
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(10);
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

        // Tanimoto elastic
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                        new SimilarityMatch<>(elasticNeedle, 0.9f))
                .collect(Collectors.toList());
        assertEquals(indigoObjectResult.canonicalSmiles(), indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
        assertFalse(bingoObjectResult.next());

    }

    @Test
    @DisplayName("Euclid test")
    public void euclid() {

        // Euclid bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "euclid-sub");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        // Euclid elastic
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                        new EuclidSimilarityMatch<>(elasticNeedle, 0.95f))
                .collect(Collectors.toList());
        assertEquals(1, indigoResult.size());
        assertEquals(indigoObjectResult.canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertFalse(bingoObjectResult.next());
    }

    @Test
    @DisplayName("Tversky test")
    public void tversky() {

        // Tversky bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "tversky");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        // Tversky elastic
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                        new TverskySimilarityMatch<>(elasticNeedle, 0.95f, 1, 1))
                .collect(Collectors.toList());
        assertEquals(indigoObjectResult.canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
        assertFalse(bingoObjectResult.next());
    }

    @Test
    @DisplayName("Exact match")
    public void exactMatch() {

        List<IndigoRecord> indigoResult = repository
                .stream()
                .filter(new ExactMatch<>(elasticNeedle))
                .limit(20)
                .collect(Collectors.toList())
                .stream()
                .filter(ExactMatch.exactMatchAfterChecker(elasticNeedle, indigo))
                .collect(Collectors.toList());

        assertEquals(elasticNeedle.getIndigoObject(indigo).canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
    }

    @Test
    @DisplayName("Substructure match")
    public void substructureMatch() {
        List<IndigoRecord> indigoResult = repository
                .stream()
                .filter(new SubstructureMatch<>(elasticNeedle))
                .limit(20)
                .collect(Collectors.toList())
                .stream()
                .filter(SubstructureMatch.substructureMatchAfterChecker(elasticNeedle, indigo))
                .collect(Collectors.toList());

        assertEquals(elasticNeedle.getIndigoObject(indigo).canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
    }

}
