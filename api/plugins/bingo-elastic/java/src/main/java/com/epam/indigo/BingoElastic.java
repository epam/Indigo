package com.epam.indigo;

import java.util.stream.Collectors;
import java.util.stream.Stream;

public class BingoElastic {

    public boolean test() {
        return false;
    }

    public static void main(String[] args) {
        IndigoRecord target = new IndigoRecord();

        Stream.of(new IndigoRecord())
                .filter(new ExactMatchPredicate<>(target))
                .filter(new RangeQueryPredicate<>("moleculeWeight", 1, 100))
                .collect(Collectors.toList()).stream().map(x -> this.test());
    }
}

