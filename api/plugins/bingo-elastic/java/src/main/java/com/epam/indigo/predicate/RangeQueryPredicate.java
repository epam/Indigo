package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;

import java.util.function.Predicate;

public class RangeQueryPredicate<T extends IndigoRecord> implements Predicate<T> {


    public RangeQueryPredicate(String field, int lowerBound, int upperBound) {
    }


    @Override
    public boolean test(T t) {
        return false;
    }
}
