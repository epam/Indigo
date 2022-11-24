package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import com.epam.indigo.predicate.SimilarityMatch;
import org.junit.Ignore;
import org.junit.jupiter.api.Test;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

/***
 * Test for local usage for OpenSearch connectivity. Ignored for now
 */
@Ignore
public class BaseLocalOpensearchTest {

    @Test
    public void connectivityTest() {
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder
                = new ElasticRepository.ElasticRepositoryBuilder<>();
        ElasticRepository<IndigoRecord> repository = builder.withIndexName(NamingConstants.BINGO_MOLECULES)
                .withHostsNames(Collections.singletonList("localhost"))
                .withPort(9201)
                .withScheme("https")
                .withUserName("admin")
                .withPassword("admin")
                .withRefreshInterval("1s")
                .withIgnoreSSL(true)
                .build();
        assertNotNull(repository);
        List<IndigoRecord> indigoResult = repository.stream().limit(10).filter(
                        new SimilarityMatch<>(Helpers.loadFromSmiles("O.O")))
                .collect(Collectors.toList());
        assertEquals(10, indigoResult.size());
    }
}
