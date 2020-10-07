package com.epam.indigo;

import com.epam.indigo.elastic.ElasticCollector;
import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatchPredicate;
import com.epam.indigo.predicate.RangeQueryPredicate;

import java.util.List;

public class BingoElastic {


    public static void main(String[] args) {
        IndigoRecord target = new IndigoRecord();

        final ElasticRepository<IndigoRecord> elasticRepository = new ElasticRepository<>();


        List<IndigoRecord> results = elasticRepository
                .stream()
                .filter(new ExactMatchPredicate<>(target))
                .filter(new RangeQueryPredicate<>("name", 1, 100))
                .collect(ElasticCollector.toList());

        for (IndigoRecord c : results) {
            System.out.println(c.getCmf());
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

