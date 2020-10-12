package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatchPredicate;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class BingoElastic {

    static Indigo indigo = new Indigo();

    public static void main(String[] args) throws IOException {
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepository.ElasticRepositoryBuilder();
        ElasticRepository<IndigoRecord> repository = builder
                .withHostName("localhost")
                .withPort(9200)
                .withScheme("http")
                .build();


        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");
        IndigoRecord indigoRecord = new IndigoRecord(indigoObject);
        indigoRecord.addCustomObject("custom_tag", "MY_FAV_MOL");
        IndigoRecord[] records = new IndigoRecord[]{indigoRecord};
        repository.indexRecords(Arrays.asList(records));

        IndigoObject targetObject = indigo.loadMolecule("C1C=CC=CC=1");
        IndigoRecord target = new IndigoRecord(targetObject);

        List<IndigoRecord> results = repository
                .stream()
                .filter(new ExactMatchPredicate<>(target))
//                .filter(new RangeQueryPredicate<>("name", 1, 100))
                .collect(Collectors.toList());

        for (IndigoRecord c : results) {
            System.out.println(c.getInternalID());
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

}

