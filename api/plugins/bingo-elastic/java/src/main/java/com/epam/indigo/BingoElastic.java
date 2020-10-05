package com.epam.indigo;

import com.epam.indigo.elastic.ElasticCollector;
import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.ExactMatchPredicate;
import com.epam.indigo.predicate.RangeQueryPredicate;

public class BingoElastic {


    public static void main(String[] args) {
        IndigoRecord target = new IndigoRecord();

        final ElasticRepository elasticRepository = new ElasticRepository();

        elasticRepository
                .stream()
                .filter(new ExactMatchPredicate<>(target))
                .filter(new RangeQueryPredicate<>("moleculeWeight", 1, 100))
                .collect(new ElasticCollector());


    }
}

