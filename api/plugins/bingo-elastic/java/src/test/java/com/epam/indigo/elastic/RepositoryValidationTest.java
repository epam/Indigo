package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

public class RepositoryValidationTest {

    @Test()
    @DisplayName("Testing not working repo")
    void testFailedRepo() {
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepository.ElasticRepositoryBuilder<>();
        builder
                .withHostName("localost")
                .withPort(9999)
                .withScheme("http");
        Assertions.assertThrows(BingoElasticException.class, builder::build);
    }


}
