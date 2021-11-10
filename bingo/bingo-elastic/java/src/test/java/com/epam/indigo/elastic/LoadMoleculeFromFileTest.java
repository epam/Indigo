package com.epam.indigo.elastic;

import com.epam.indigo.Bingo;
import com.epam.indigo.BingoObject;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.NamingConstants;
import com.epam.indigo.predicate.SimilarityMatch;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;
import org.testcontainers.utility.DockerImageName;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class LoadMoleculeFromFileTest {

    protected static ElasticRepository<IndigoRecordMolecule> repository;
    private static ElasticsearchContainer elasticsearchContainer;
    private static Bingo bingoDb;
    private static Indigo indigo;

    @BeforeAll
    public static void setUpElastic() {
        elasticsearchContainer = new ElasticsearchContainer(
                DockerImageName
                        .parse(ElasticsearchVersion.DOCKER_IMAGE_NAME)
                        .withTag(ElasticsearchVersion.VERSION)
        );
        elasticsearchContainer.start();
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecordMolecule> builder = new ElasticRepository.ElasticRepositoryBuilder<>();
        repository = builder
                .withIndexName(NamingConstants.BINGO_MOLECULES)
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
    public static void tearDownBingoNoSQL() throws Throwable {
//        indigo.finalize();
    }

    @BeforeEach
    public void setUpBingoNoSQL() {
        indigo = new Indigo();
        bingoDb = Bingo.createDatabaseFile(indigo, "src/test/resources/bingo_nosql", "molecule");
    }

    @AfterEach
    public void deleteIndex() throws IOException {
        repository.deleteAllRecords();
        bingoDb.close();
    }

    /**
     * Use this method to test additional/custom fields loaded into record
     */
    protected void testAdditionalFields(IndigoRecord record, HashMap<String, String> requiredFields) {
        Assertions.fail();
    }


    @Test
    @DisplayName("Testing creation of IndigoRecord from mol file")
    void testLoadFromMol() {
        IndigoRecordMolecule indigoRecord = Helpers.loadMolecule("src/test/resources/composition1.mol");
        assertNotNull(indigoRecord.getSimFingerprint());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from cml file")
    public void testLoadFromCml() {
        List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
        Helpers.iterateCml("src/test/resources/tetrahedral-all.cml").forEach(indigoRecordList::add);
        assertEquals(163, indigoRecordList.size());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from cml file with name")
    public void testLoadFromCmlWithName() throws Exception {
        List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
        Helpers.iterateCml("src/test/resources/tetrahedral-named.cml").forEach(indigoRecordList::add);
        repository.indexRecords(indigoRecordList, indigoRecordList.size());
        TimeUnit.SECONDS.sleep(5);
        List<IndigoRecord> indigoRecordResult = repository.stream().collect(Collectors.toList());
        assertEquals(1, indigoRecordList.size());
        assertEquals("tetrahedralTitle", indigoRecordList.get(0).getName());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from sdf file")
    void testLoadFromSdf() throws Exception {
        List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
        Helpers.iterateSdf("src/test/resources/rand_queries_small.sdf").forEach(indigoRecordList::add);
        assertEquals(371, indigoRecordList.size());
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from sdf file with names")
    void testLoadFromSdfWithName() throws Exception {
        List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
        Helpers.iterateSdf("src/test/resources/zinc-slice.sdf.gz").forEach(indigoRecordList::add);
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
            IndigoRecordMolecule indigoRecord = Helpers.loadFromSmiles(smiles);
            repository.indexRecord(indigoRecord);
            IndigoRecord indigoTestRecord = Helpers.loadFromSmiles(smiles);
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(indigoTestRecord, 1))
                    .collect(Collectors.toList());
            assertEquals(1, similarRecords.size());

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @Test
    @DisplayName("Testing indexing and retrieving exact match for empty fingerprint molecules")
    void testExactMatchOnEmptyFingerprint() {
        try {
            String smiles1 = "[H][H]";
            IndigoRecordMolecule indigoRecord1 = Helpers.loadFromSmiles(smiles1);
            String smiles2 = "[H][H][H]";
            IndigoRecordMolecule indigoRecord2 = Helpers.loadFromSmiles(smiles2);
            repository.indexRecord(indigoRecord1);
            repository.indexRecord(indigoRecord2);
            TimeUnit.SECONDS.sleep(5);

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @Test
    @DisplayName("Testing indexing and retrieving similar match for empty fingerprint molecules")
    void testSimMatchOnEmptyFingerprint() {
        try {
            String smiles1 = "[H][H]";
            IndigoRecordMolecule indigoRecord1 = Helpers.loadFromSmiles(smiles1);
            String smiles2 = "[H][H][H]";
            IndigoRecordMolecule indigoRecord2 = Helpers.loadFromSmiles(smiles2);
            repository.indexRecord(indigoRecord1);
            repository.indexRecord(indigoRecord2);
            TimeUnit.SECONDS.sleep(5);

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles file")
    void testLoadFromSmiFile() {
        String needle = "SCC(NC(=O)CCNC(=O)C(O)C(COP(O)(O)=O)(C)C)C(O)=O";
        try {
            String testFile = "src/test/resources/pubchem_slice_50.smi";
            List<IndigoRecordMolecule> indigoRecordList = new ArrayList<>();
            Helpers.iterateSmiles(testFile).forEach(indigoRecordList::add);
            assertEquals(50, indigoRecordList.size());
            IndigoRecordMolecule indigoTestRecord = Helpers.loadFromSmiles(needle);
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(5);

            List<IndigoRecordMolecule> similarRecords = repository.stream()
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
            IndigoRecordMolecule elasticFound = similarRecords.get(0);
            IndigoObject indigoElasticFound = indigo.deserialize(elasticFound.getCmf());
            assertEquals(indigo.similarity(bingoFound, indigoElasticFound), 1.0f);

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

}
