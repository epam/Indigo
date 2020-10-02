package com.epam.indigo;

import java.util.function.Predicate;

public class RangeQueryPredicate<T extends IndigoRecord> implements Predicate<T> {


    public RangeQueryPredicate(String moleculeWeight, int start, int finish) {
    }

    @Override
    public boolean test(Object o) {
        return true;
    }
}
