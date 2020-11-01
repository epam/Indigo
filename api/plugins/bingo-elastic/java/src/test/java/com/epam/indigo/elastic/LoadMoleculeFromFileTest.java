package com.epam.indigo.elastic;

import com.epam.indigo.Bingo;
import com.epam.indigo.BingoObject;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.SimilarityMatch;
import org.elasticsearch.ElasticsearchStatusException;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class LoadMoleculeFromFileTest {

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


    /**
     * Use this method to test additional/custom fields loaded into record
     */
    protected void testAdditionalFields(IndigoRecord record, HashMap<String, String> requiredFields) {
        Assertions.fail();
    }


    @Test
    @DisplayName("Testing creation of IndigoRecord from mol file")
    void testLoadFromMol() throws Exception {
        IndigoRecord indigoRecord = Helpers.loadFromFile("src/test/resources/composition1.mol");
        assertNotNull(indigoRecord.getSimFingerprint());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from cml file")
    public void testLoadFromCml() throws Exception {
        List<IndigoRecord> indigoRecordList = Helpers.loadFromCmlFile("src/test/resources/tetrahedral-all.cml");
        assertEquals(163, indigoRecordList.size());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from cml file with name")
    public void testLoadFromCmlWithName() throws Exception {
        List<IndigoRecord> indigoRecordList = Helpers.loadFromCmlFile("src/test/resources/tetrahedral-named.cml");
        repository.indexRecords(indigoRecordList, indigoRecordList.size());
        TimeUnit.SECONDS.sleep(5);
        List<IndigoRecord> indigoRecordResult = repository.stream().collect(Collectors.toList());
        assertEquals(1, indigoRecordList.size());
        assertEquals("tetrahedralTitle", indigoRecordList.get(0).getName());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from sdf file")
    void testLoadFromSdf() throws Exception {
        List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
        assertEquals(371, indigoRecordList.size());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from sdf file with names")
    void testLoadFromSdfWithName() throws Exception {
        List<IndigoRecord> indigoRecordList =
                Helpers.loadFromSdf("src/test/resources/zinc-slice.sdf.gz");
        repository.indexRecord(indigoRecordList.get(0));
        assertEquals(721, indigoRecordList.size());
        TimeUnit.SECONDS.sleep(5);
        List<IndigoRecord> indigoRecordResult = repository.stream()
                .limit(1).collect(Collectors.toList());
        assertEquals("ZINC03099968", indigoRecordResult.get(0).getName());
    }


    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles")
    void testLoadFromSmilesString() {
        try {
            String smiles = "O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C";
            IndigoRecord indigoRecord = Helpers.loadFromSmiles(smiles);
            repository.indexRecord(indigoRecord);
            IndigoRecord indigoTestRecord = Helpers.loadFromSmiles(smiles);
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(indigoTestRecord, 1))
                    .collect(Collectors.toList());
            assertEquals(1, similarRecords.size());

        } catch (Exception e) {
            Assertions.fail();
        }
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles file")
    void testLoadFromSmiFile() {
        String needle = "SCC(NC(=O)CCNC(=O)C(O)C(COP(O)(O)=O)(C)C)C(O)=O";
        try {
            String testFile = "src/test/resources/pubchem_slice_50.smi";

            List<IndigoRecord> indigoRecordList = Helpers.loadFromSmilesFile(
                    testFile
            );
            assertEquals(50, indigoRecordList.size());
            IndigoRecord indigoTestRecord = Helpers.loadFromSmiles(needle);
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(5);

            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(indigoTestRecord, 1))
                    .limit(1)
                    .collect(Collectors.toList());

            for (IndigoObject indigoObject : indigo.iterateSmilesFile(testFile)) {
                bingoDb.insert(indigoObject);
            }

            // Similar
            BingoObject result = bingoDb.searchSim(indigo.loadMolecule(needle), 1, 1);
            result.next();
            IndigoObject bingoFound = result.getIndigoObject();
            IndigoRecord elasticFound = similarRecords.get(0);
            IndigoObject indigoElasticFound = indigo.unserialize(elasticFound.getCmf());
            assertEquals(indigo.similarity(bingoFound, indigoElasticFound), 1.0f);

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

}
