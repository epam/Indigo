package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatch;
import com.epam.indigo.predicate.TanimotoSimilarityMatch;
import org.elasticsearch.ElasticsearchStatusException;
import org.junit.jupiter.api.*;
import static org.junit.jupiter.api.Assertions.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

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

    @BeforeEach
    public void setUpBingoNoSQL() {
        indigo = new Indigo();
        bingoDb = Bingo.createDatabaseFile(indigo, "src/test/resources/bingo_nosql", "molecule");
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @AfterAll
    public static void tearDownBingoNoSQL() {

    }

    @AfterEach
    public void deleteIndex() throws IOException {
        try {
            repository.deleteAllRecords();
        } catch (ElasticsearchStatusException ignored) {

        }
    }

    @Test
    @DisplayName("Compare results from CDF")
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

        // Tanimoto
        List<IndigoRecord> indigoResult = repository.stream().limit(1).filter(
                new ExactMatch<>(elasticNeedle)).collect(Collectors.toList());


        BingoObject bingoObjectResult = bingoDb.searchExact(bingoNeedle);
        bingoObjectResult.next();
        IndigoObject indigoObjectResult = bingoObjectResult.getIndigoObject();

        assertEquals(indigoObjectResult.oneBitsList(),
                indigo.loadMoleculeFromBuffer(indigoResult.get(0).getCmf()).oneBitsList());
    }

}
