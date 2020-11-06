package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.elastic.ElasticRepository.ElasticRepositoryBuilder;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecord.IndigoRecordBuilder;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class SaveMoleculeFromIndigoRecordTest {

    protected static ElasticRepository<IndigoRecord> repository;
    private static ElasticsearchContainer elasticsearchContainer;


    @BeforeAll
    public static void setUpElastic() {
        elasticsearchContainer = new ElasticsearchContainer("docker.elastic.co/elasticsearch/elasticsearch-oss:7.9.2");
        elasticsearchContainer.start();
        ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepositoryBuilder<>();
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

    @AfterEach
    public void deleteIndex() throws IOException {
        repository.deleteAllRecords();
    }

    @Test
    @DisplayName("Testing saving IndigoRecord from File")
    public void saveFromFile() {
        IndigoRecord indigoRecord = Helpers.loadFromFile("src/test/resources/composition1.mol");
        try {
            repository.indexRecord(indigoRecord);
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> collect = repository.stream().collect(Collectors.toList());
            assertEquals(1, collect.size());
        } catch (Exception exception) {
            Assertions.fail(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from sdf file")
    public void saveFromSdfFile() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> collect = repository.stream().collect(Collectors.toList());
            assertEquals(10, collect.size());
        } catch (Exception exception) {
            Assertions.fail(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from cml file")
    public void saveFromCmlFile() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromCmlFile("src/test/resources/tetrahedral-all.cml");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> collect = repository.stream().collect(Collectors.toList());
            assertEquals(10, collect.size());
        } catch (Exception exception) {
            Assertions.fail(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from smiles")
    public void saveFromSmiles() {
        try {
            IndigoRecord indigoRecord = Helpers.loadFromSmiles("O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C");
            repository.indexRecord(indigoRecord);
            TimeUnit.SECONDS.sleep(5);
            List<IndigoRecord> collect = repository.stream().collect(Collectors.toList());
            assertEquals(1, collect.size());
        } catch (Exception exception) {
            Assertions.fail(exception);
        }
    }

    @Test
    @DisplayName("Test empty molecule save")
    public void saveEmptyMolecule() {
        Indigo session = new Indigo();
        IndigoObject mol = session.createMolecule();
        Exception exception = assertThrows(BingoElasticException.class, () -> {
            (new IndigoRecordBuilder()).withIndigoObject(mol).build();
        });
        String expectedMessage = "Building IndigoRecords from empty IndigoObject is not supported";
        String actualMessage = exception.getMessage();
        assertTrue(actualMessage.contains(expectedMessage));

    }

}
