package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.util.List;

public class SaveMoleculeFromIndigoRecordTest {

    protected static ElasticRepository<IndigoRecord> repository;

    @BeforeAll
    public static void setUpElastic() {
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepository.ElasticRepositoryBuilder();
        repository = builder
                .withHostName("localhost")
                .withPort(9200)
                .withScheme("http")
                .build();
    }

    @AfterAll
    public static void tearDownElastic() {

    }

    @Test
    @DisplayName("Testing saving IndigoRecord from File")
    public void saveFromFile() {
        IndigoRecord indigoRecord = Helpers.loadFromFile("src/test/resources/composition1.mol");
        try {
            repository.indexRecord(indigoRecord);
        } catch (IOException exception) {
            System.out.println(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from sdf file")
    public void saveFromSdfFile() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
        } catch (Exception exception) {
            System.out.println(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from cml file")
    public void saveFromCmlFile() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromCmlFile("src/test/resources/tetrahedral-all.cml");
            repository.indexRecords(indigoRecordList);
        } catch (Exception exception) {
            System.out.println(exception);
        }
    }

    @Test
    @DisplayName("Testing saving from smiles")
    public void saveFromSmiles() {
        try {
            IndigoRecord indigoRecord = Helpers.loadFromSmiles("O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C");
            repository.indexRecord(indigoRecord);
        } catch (Exception exception) {
            System.out.println(exception);
        }
    }

}
