package com.epam.indigo.elastic;

import com.epam.indigo.Bingo;
import com.epam.indigo.BingoObject;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatch;
import com.epam.indigo.predicate.SimilarityMatch;
import org.elasticsearch.ElasticsearchStatusException;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.admin.indices.refresh.RefreshResponse;
import org.elasticsearch.action.support.WriteRequest.RefreshPolicy;
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
                .withRefreshPolicy(RefreshPolicy.IMMEDIATE)
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
        repository.deleteAllRecords();
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
        repository.refreshIndex(new ActionListener<RefreshResponse>() {
            @Override
            public void onResponse(RefreshResponse refreshResponse) {
                List<IndigoRecord> indigoRecordResult = repository.stream().collect(Collectors.toList());
                assertEquals(1, indigoRecordList.size());
                assertEquals("tetrahedralTitle", indigoRecordList.get(0).getName());
            }

            @Override
            public void onFailure(Exception e) {
                Assertions.fail(e);
            }
        });
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

        repository.refreshIndex(new ActionListener<RefreshResponse>() {
            @Override
            public void onResponse(RefreshResponse refreshResponse) {
                List<IndigoRecord> indigoRecordResult = repository.stream()
                        .limit(1).collect(Collectors.toList());
                assertEquals("ZINC03099968", indigoRecordResult.get(0).getName());
            }
            @Override
            public void onFailure(Exception e) {
                Assertions.fail(e);
            }
        });
    }


    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles")
    void testLoadFromSmilesString() {
        try {
            String smiles = "O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C";
            IndigoRecord indigoRecord = Helpers.loadFromSmiles(smiles);
            repository.indexRecord(indigoRecord);
            repository.refreshIndex(new ActionListener<RefreshResponse>() {
                @Override
                public void onResponse(RefreshResponse refreshResponse) {
                    IndigoRecord indigoTestRecord = Helpers.loadFromSmiles(smiles);
                    List<IndigoRecord> similarRecords = repository.stream()
                            .filter(new SimilarityMatch<>(indigoTestRecord, 1))
                            .collect(Collectors.toList());
                    assertEquals(1, similarRecords.size());
                }

                @Override
                public void onFailure(Exception e) {
                    Assertions.fail();
                }
            });

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @Test
    @DisplayName("Testing indexing and retrieving exact match for empty fingerprint molecules")
    void testExactMatchOnEmptyFingerprint() {
        try {
            String smiles1 = "[H][H]";
            IndigoRecord indigoRecord1 = Helpers.loadFromSmiles(smiles1);
            String smiles2 = "[H][H][H]";
            IndigoRecord indigoRecord2 = Helpers.loadFromSmiles(smiles2);
            repository.indexRecord(indigoRecord1);
            repository.indexRecord(indigoRecord2);
            repository.refreshIndex(new ActionListener<RefreshResponse>() {
                @Override
                public void onResponse(RefreshResponse refreshResponse) {
                    List<IndigoRecord> exactMatchRecords = repository.stream()
                            .filter(new ExactMatch<>(indigoRecord1))
                            .collect(Collectors.toList())
                            .stream()
                            .filter(ExactMatch.exactMatchAfterChecker(indigoRecord1, indigo))
                            .collect(Collectors.toList());
                    assertEquals(1, exactMatchRecords.size());
                    assertEquals(smiles1, indigo.loadMolecule(exactMatchRecords.get(0).getCmf()).canonicalSmiles());
                }

                @Override
                public void onFailure(Exception e) {
                    Assertions.fail(e);
                }
            });

        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

    @Test
    @DisplayName("Testing indexing and retrieving similar match for empty fingerprint molecules")
    void testSimMatchOnEmptyFingerprint() {
        try {
            String smiles1 = "[H][H]";
            IndigoRecord indigoRecord1 = Helpers.loadFromSmiles(smiles1);
            String smiles2 = "[H][H][H]";
            IndigoRecord indigoRecord2 = Helpers.loadFromSmiles(smiles2);
            repository.indexRecord(indigoRecord1);
            repository.indexRecord(indigoRecord2);
            repository.refreshIndex(new ActionListener<RefreshResponse>() {
                @Override
                public void onResponse(RefreshResponse refreshResponse) {
                    List<IndigoRecord> simMatchRecords = repository.stream()
                            .filter(new SimilarityMatch<>(indigoRecord1))
                            .collect(Collectors.toList());
                    assertEquals(1, simMatchRecords.size());
                    assertEquals(smiles1, indigo.loadMolecule(simMatchRecords.get(0).getCmf()).canonicalSmiles());
                }

                @Override
                public void onFailure(Exception e) {
                    Assertions.fail(e);
                }
            });

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

            List<IndigoRecord> indigoRecordList = Helpers.loadFromSmilesFile(
                    testFile
            );
            assertEquals(50, indigoRecordList.size());
            IndigoRecord indigoTestRecord = Helpers.loadFromSmiles(needle);
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            repository.refreshIndex(new ActionListener<RefreshResponse>() {
                @Override
                public void onResponse(RefreshResponse refreshResponse) {
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
                    IndigoObject indigoElasticFound = indigo.deserialize(elasticFound.getCmf());
                    assertEquals(indigo.similarity(bingoFound, indigoElasticFound), 1.0f);
                }

                @Override
                public void onFailure(Exception e) {
                    Assertions.fail(e);
                }
            });


        } catch (Exception e) {
            Assertions.fail(e);
        }
    }

}
