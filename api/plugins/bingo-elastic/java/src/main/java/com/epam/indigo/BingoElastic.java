package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.elastic.ElasticRepository.ElasticRepositoryBuilder;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatch;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Example of usage for Bingo Elastic
 */
public class BingoElastic {

    public static void main(String[] args) throws Exception {
        ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepositoryBuilder<>();
        ElasticRepository<IndigoRecord> repository = builder
                .withHostName("localhost")
                .withPort(9200)
                .withScheme("http")
                .build();


        IndigoRecord indigoRecord = Helpers.loadFromSmiles("C1=CC=CC=C1");
        indigoRecord.addCustomObject("custom_tag", "MY_FAV_MOL");
        IndigoRecord[] records = new IndigoRecord[]{indigoRecord};
        repository.indexRecords(Arrays.asList(records));

        IndigoRecord target = Helpers.loadFromSmiles("C1C=CC=CC=1");

        List<IndigoRecord> results = repository
                .stream()
                .filter(new ExactMatch<>(target))
//                .filter(new RangeQueryPredicate<>("name", 1, 100))
                .collect(Collectors.toList());

        for (IndigoRecord c : results) {
            System.out.println(c.getInternalID());
        }

    }
}


//    1. Provide code generators from fav formats? from sdf
//    2. support loader from sdf/mol/...? formats & support by default wildcard & exact text/string match
//    3. autodetect - digits, text?

//    text -> exact match, wildcard, "novartis department"~2
//    digits -> exact match, range
//    dates -> exact match, range

//    add kinda schema for sanity checks on field types
//



