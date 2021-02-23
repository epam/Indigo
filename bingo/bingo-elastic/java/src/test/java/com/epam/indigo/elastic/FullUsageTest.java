package com.epam.indigo.elastic;

import com.epam.indigo.Indigo;
import com.epam.indigo.elastic.ElasticRepository.ElasticRepositoryBuilder;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.*;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.List;
import java.util.Random;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class FullUsageTest {

    private static final Random random = new Random();
    protected static ElasticRepository<IndigoRecord> repository;
    private static ElasticsearchContainer elasticsearchContainer;
    private static Indigo indigo;

    @BeforeAll
    public static void setUpElastic() {
        indigo = new Indigo();
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
    @DisplayName("Testing full usage, indexing, searching")
    public void fullUsage() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(10);
            int requestSize = 20;
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(target, 0.8f))
                    .limit(requestSize)
                    .collect(Collectors.toList());
            assertEquals(1.0f, similarRecords.get(0).getScore());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing exact match")
    public void exactMatch() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(10);
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new ExactMatch<>(target))
                    .limit(20)
                    .collect(Collectors.toList())
                    .stream()
                    .filter(ExactMatch.exactMatchAfterChecker(target, indigo))
                    .collect(Collectors.toList());
            assertEquals(1, similarRecords.size());
            assertEquals(1.0f, similarRecords.get(0).getScore());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }


    @Test
    @DisplayName("Testing tversky match")
    public void tversky() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(10);
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            float threshold = 0.8f;
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new TverskySimilarityMatch<>(target, threshold, 0.5f, 0.5f))
                    .collect(Collectors.toList());
            for (IndigoRecord similarRecord : similarRecords) assertTrue(similarRecord.getScore() >= threshold);
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing euclid with threshold match")
    public void euclidWithThreshold() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(5);
            float threshold = 0.5f;
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new EuclidSimilarityMatch<>(target, threshold))
                    .collect(Collectors.toList());

            for (IndigoRecord similarRecord : similarRecords) assertTrue(similarRecord.getScore() >= threshold);
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing tanimoto and keyword query")
    public void keywordQueryWithTanimoto() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            IndigoRecord indigoRecord = indigoRecordList.get(0);
            String test = "Test";
            String fieldName = "tag";
            indigoRecord.addCustomObject(fieldName, test);
            repository.indexRecords(indigoRecordList, 100);
            TimeUnit.SECONDS.sleep(10);
            IndigoRecord target = indigoRecordList.get(0);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(target))
                    .filter(new KeywordQuery<>(fieldName, test))
                    .collect(Collectors.toList());

            assertEquals(1, similarRecords.size());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing tanimoto and range query")
    public void rangeQueryWithTanimoto() {
        try {
            Random r = new Random();
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            String fieldName = "weight";
            int cnt = 0;
            for (IndigoRecord rec: indigoRecordList) {
                int weight = r.nextInt(1000);
                if (weight >= 10 && weight <= 100)
                    cnt++;
                rec.addCustomObject(fieldName, weight);
            }
            repository.indexRecords(indigoRecordList, 100);
            TimeUnit.SECONDS.sleep(10);
            IndigoRecord target = indigoRecordList.get(0);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(target))
                    .filter(new RangeQuery<>(fieldName, 10, 100))
                    .collect(Collectors.toList());

            assertEquals(Math.min(10, cnt), similarRecords.size());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing substructure search")
    public void substructureSearch() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList, indigoRecordList.size());
            TimeUnit.SECONDS.sleep(10);
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> records = repository.stream()
                    .filter(new SubstructureMatch<>(target))
                    .limit(20)
                    .collect(Collectors.toList())
                    .stream()
                    .filter(SubstructureMatch.substructureMatchAfterChecker(target, indigo))
                    .collect(Collectors.toList());

            assertEquals(1, records.size());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing RangeQuery search")
    public void rangeSearch() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            int i = 0;
            for (IndigoRecord record : indigoRecordList) {
                record.addCustomObject("ind_number", i++);
            }
            repository.indexRecords(indigoRecordList, 1000);
            TimeUnit.SECONDS.sleep(10);
            List<IndigoRecord> records = repository.stream()
                                .filter(new RangeQuery<>("ind_number", 1, 10))
                                .limit(20)
                                .collect(Collectors.toList());
            assertEquals(records.size(), 10);
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing WildcardQuery search")
    public void wildcardSearch() {

        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            IndigoRecord indigoRecord = indigoRecordList.get(0);
            String test = "Test";
            String fieldName = "tag";
            indigoRecord.addCustomObject(fieldName, test);
            repository.indexRecords(indigoRecordList, 100);
            TimeUnit.SECONDS.sleep(10);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new WildcardQuery<>(fieldName, "T*t"))
                    .collect(Collectors.toList());

            assertEquals(1, similarRecords.size());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }

    }
}
