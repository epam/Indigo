package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.EuclidSimilarityMatch;
import com.epam.indigo.predicate.ExactMatch;
import com.epam.indigo.predicate.TanimotoSimilarityMatch;
import com.epam.indigo.predicate.TverskySimilarityMatch;
import org.elasticsearch.ElasticsearchStatusException;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class BingoNoSQLVersusElasticTest {

    protected static ElasticRepository<IndigoRecord> repository;
    private static ElasticsearchContainer elasticsearchContainer;
    private static Bingo bingoDb;
    private static Indigo indigo;

    @BeforeAll
    public static void setUpElastic() {
        elasticsearchContainer = new ElasticsearchContainer("docker.elastic.co/elasticsearch/elasticsearch-oss:7.9.2");
        elasticsearchContainer.start();
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepository.ElasticRepositoryBuilder<>();
        repository = builder
                .withHostName(elasticsearchContainer.getHost())
                .withPort(elasticsearchContainer.getFirstMappedPort())
                .withScheme("http")
                .withRefreshInterval("1s")
                .build();
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @AfterAll
    public static void tearDownBingoNoSQL() {

    }

    @BeforeEach
    public void setUpBingoNoSQL() {
        indigo = new Indigo();
        bingoDb = Bingo.createDatabaseFile(indigo, "src/test/resources/bingo_nosql", "molecule");
    }

    @AfterEach
    public void deleteIndex() throws IOException {
        try {
            repository.deleteAllRecords();
        } catch (ElasticsearchStatusException ignored) {

        }
    }

    @Test
    @DisplayName("Compare results by similarity and exact match (compound exists)")
    public void compareResultsFromCDF() throws Exception {
        String smiles = "CC(=C)C(=O)NC1C=CC=CC=1C([O-])=O";
        IndigoObject bingoNeedle = indigo.loadMolecule(smiles);
        IndigoRecord elasticNeedle = Helpers.loadFromSmiles(smiles);
        String testFile = "src/test/resources/zinc-slice.sdf";
        List<IndigoRecord> indigoRecordList =
                Helpers.loadFromSdf(testFile);
        repository.indexRecords(indigoRecordList);
        TimeUnit.SECONDS.sleep(5);

        for (IndigoObject indigoObject : indigo.iterateSDFile(testFile)) {
            bingoDb.insert(indigoObject);
        }

        // Exact match
        List<IndigoRecord> indigoResult = repository.stream().limit(1).filter(
                new ExactMatch<>(elasticNeedle)).collect(Collectors.toList());

        assertTrue(indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles().equals(smiles));

        // Tanimoto bingo
        BingoObject bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.9f, 1, "tanimoto");
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertTrue(indigoObjectResult.canonicalSmiles().equals("CC(=C)C(=O)Nc1ccccc1C([O-])=O"));
        assertFalse(bingoObjectResult.next());

        // Tanimoto elastic
        indigoResult = repository.stream().limit(10).filter(
                new TanimotoSimilarityMatch<>(Helpers.loadFromSmiles(smiles), 0.99f))
                .collect(Collectors.toList());
        assertTrue(indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles().equals(smiles));
        assertEquals(1, indigoResult.size());

        // Euclid bingo
        bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "euclid-sub");
        bingoObjectResult.next();
        indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertTrue(indigoObjectResult.canonicalSmiles().equals("CC(=C)C(=O)Nc1ccccc1C([O-])=O"));
        assertFalse(bingoObjectResult.next());

        // Euclid elastic
        indigoResult = repository.stream().limit(10).filter(
                new EuclidSimilarityMatch<>(Helpers.loadFromSmiles(smiles), 0.95f))
                .collect(Collectors.toList());
        assertTrue(indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles().equals(smiles));
        assertEquals(6, indigoResult.size());

        // Tversky bingo
        bingoObjectResult = bingoDb.searchSim(bingoNeedle, 0.95f, 1, "euclid-sub");
        bingoObjectResult.next();
        indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertTrue(indigoObjectResult.canonicalSmiles().equals("CC(=C)C(=O)Nc1ccccc1C([O-])=O"));
        assertFalse(bingoObjectResult.next());

        // Tversky elastic
        indigoResult = repository.stream().limit(10).filter(
                new TverskySimilarityMatch<>(Helpers.loadFromSmiles(smiles), 1, 1, 1))
                .collect(Collectors.toList());
        assertTrue(indigoResult.get(0).getIndigoObject(indigo).canonicalSmiles().equals(smiles));
        assertEquals(1, indigoResult.size());

    }

}
