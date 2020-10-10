package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;

import java.util.function.Predicate;

public class ExactMatchPredicate<T extends IndigoRecord> implements Predicate<T> {

    private final T target;

    public ExactMatchPredicate(T target) {
        this.target = target;
    }

    @Override
    public boolean test(T t) {
//        todo check with industry wise lazy predicates
        return true;
    }
}
