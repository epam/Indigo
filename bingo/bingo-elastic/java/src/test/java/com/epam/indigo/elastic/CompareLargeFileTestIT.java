package com.epam.indigo.elastic;

import com.epam.indigo.BingoObject;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.predicate.*;
import org.elasticsearch.common.collect.Tuple;
import org.junit.jupiter.api.*;

import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

public class CompareLargeFileTestIT extends NoSQLElasticCompareAbstract {

    protected static final String test100SmilesFile = "src/test/resources/pubchem_slice_100000.smiles";

    protected static final String[] smiles = new String[]{
            "CC(=C)C(=O)NC1C=CC=CC=1C([O-])=O",
            "NC(C=CC=CCCCCCCCC)C"
    };

    private static void loadFile() {
        long noSQLTotal = System.nanoTime();
        for (IndigoObject indigoObject : indigo.iterateSmilesFile(test100SmilesFile)) {
            bingoDb.insert(indigoObject);
        }
        noSQLTotal = System.nanoTime() - noSQLTotal;
        long elasticTotal = System.nanoTime();
        try {
            Iterable<IndigoRecordMolecule> indigoRecordList = Helpers.iterateSmiles(test100SmilesFile);
            repository.indexRecords(indigoRecordList, 5000);
        } catch (Exception e) {
            Assertions.fail(e);
        }
        elasticTotal = System.nanoTime() - elasticTotal;

        try {
            TimeUnit.SECONDS.sleep(180);
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

    protected List<Tuple<String, Float>> bingoNoSQLSimilarity(String similarity, IndigoObject bingoNeedle, float threshold) {

        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, threshold, 1, similarity);

        List<Tuple<String, Float>> nosqlListResult = new ArrayList<>();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();
        while (bingoObjectResult.next()) {
            String bingoFoundSmiles = indigoObjectResult.canonicalSmiles();
            nosqlListResult.add(new Tuple<>(bingoFoundSmiles, bingoObjectResult.getCurrentSimilarityValue()));
        }
        nosqlListResult.sort((o1, o2) -> -Float.compare(o1.v2(), o2.v2()));
        return nosqlListResult;
    }

    protected List<Tuple<String, Float>> elasticSimilarity(BaseMatch<IndigoRecord> similarity, IndigoRecord elasticNeedle) {
        List<Tuple<String, Float>> elasticListResult = new ArrayList<>();
        List<IndigoRecord> elasticResults = repository.stream().limit(1000)
                .filter(similarity)
                .collect(Collectors.toList());
        for (IndigoRecord indigoRecordResult : elasticResults) {
            Tuple<String, Float> elasticTuple = new Tuple<>(indigoRecordResult.getIndigoObject(indigo).canonicalSmiles(), indigoRecordResult.getScore());
            elasticListResult.add(elasticTuple);
        }
        return elasticListResult;
    }

    @Test
    @DisplayName("Tanimoto test")
    public void tanimoto() {
        for (String curSmiles : smiles) {
            IndigoObject bingoNeedle = indigo.loadMolecule(curSmiles);
            IndigoRecord elasticNeedle = Helpers.loadFromSmiles(curSmiles);
            float threshold = 0.7f;

            List<Tuple<String, Float>> elasticListResult = elasticSimilarity(new SimilarityMatch<>(elasticNeedle, threshold), elasticNeedle);
            List<Tuple<String, Float>> nosqlListResult = bingoNoSQLSimilarity("tanimoto", bingoNeedle, threshold);

            assertEquals(elasticListResult.size(), nosqlListResult.size());
        }
    }

    @Test
    @DisplayName("Euclid test")
    public void euclid() {
        for (String curSmiles : smiles) {
            IndigoObject bingoNeedle = indigo.loadMolecule(curSmiles);
            IndigoRecord elasticNeedle = Helpers.loadFromSmiles(curSmiles);
            float threshold = 0.9f;

            List<Tuple<String, Float>> elasticListResult = elasticSimilarity(new EuclidSimilarityMatch<>(elasticNeedle, threshold), elasticNeedle);
            List<Tuple<String, Float>> nosqlListResult = bingoNoSQLSimilarity("euclid-sub", bingoNeedle, threshold);
            assertEquals(elasticListResult.size(), nosqlListResult.size());
        }
    }

    @Test
    @DisplayName("Tversky test")
    public void tversky() {
        for (String curSmiles : smiles) {
            IndigoObject bingoNeedle = indigo.loadMolecule(curSmiles);
            IndigoRecord elasticNeedle = Helpers.loadFromSmiles(curSmiles);
            float threshold = 0.9f;

            List<Tuple<String, Float>> elasticListResult = elasticSimilarity(new TverskySimilarityMatch<>(elasticNeedle, threshold, 0.5f, 0.5f), elasticNeedle);
            List<Tuple<String, Float>> nosqlListResult = bingoNoSQLSimilarity("tversky", bingoNeedle, threshold);
            assertEquals(elasticListResult.size(), nosqlListResult.size());
        }
    }

    @Test
    @DisplayName("Exact match")
    public void exactMatch() {

        IndigoRecord elasticNeedle = Helpers.loadFromSmiles(smiles[1]);
        IndigoObject bingoNeedle = indigo.loadMolecule(smiles[1]);
        long noSQLTotal = System.nanoTime();
        BingoObject bingoObjectResult = bingoDb.searchExact(bingoNeedle);
        noSQLTotal = System.nanoTime() - noSQLTotal;
        assertTrue(bingoObjectResult.next());
        assertEquals(indigo.loadMolecule(smiles[1]).canonicalSmiles(),
                bingoObjectResult.getIndigoObject().canonicalSmiles());

        long elasticTotal = System.nanoTime();

        List<IndigoRecord> indigoResult = repository
                .stream()
                .filter(new ExactMatch<>(elasticNeedle))
                .limit(20)
                .collect(Collectors.toList())
                .stream()
                .filter(ExactMatch.exactMatchAfterChecker(elasticNeedle, indigo))
                .collect(Collectors.toList());
        elasticTotal = System.nanoTime() - elasticTotal;

        assertEquals(indigo.loadMolecule(smiles[1]).canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
        assertTrue(noSQLTotal < elasticTotal);
    }

    @Test
    @DisplayName("Substructure match")
    public void substructureMatch() {
        IndigoRecord elasticNeedle = Helpers.loadFromSmiles(smiles[1]);
        IndigoObject bingoNeedle = indigo.loadQueryMolecule(smiles[1]);
        long noSQLTotal = System.nanoTime();
        BingoObject bingoObjectResult = bingoDb.searchSub(bingoNeedle);
        noSQLTotal = System.nanoTime() - noSQLTotal;
        assertTrue(bingoObjectResult.next());
        assertEquals(indigo.loadMolecule(smiles[1]).canonicalSmiles(),
                bingoObjectResult.getIndigoObject().canonicalSmiles());

        long elasticTotal = System.nanoTime();

        List<IndigoRecord> indigoResult = repository
                .stream()
                .filter(new SubstructureMatch<>(elasticNeedle))
                .limit(20)
                .collect(Collectors.toList())
                .stream()
                .filter(SubstructureMatch.substructureMatchAfterChecker(elasticNeedle, indigo))
                .collect(Collectors.toList());
        elasticTotal = System.nanoTime() - elasticTotal;

        assertEquals(indigo.loadMolecule(smiles[1]).canonicalSmiles(),
                indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles());
        assertEquals(1, indigoResult.size());
        assertTrue(noSQLTotal < elasticTotal);
    }
}
