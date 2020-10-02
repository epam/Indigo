package com.epam.indigo;

import java.util.function.Predicate;

public class ExactMatchPredicate<T extends IndigoRecord> implements Predicate<T> {

    private final T target;

    public ExactMatchPredicate(T target) {
        this.target = target;
    }

    @Override
    public boolean test(T t) {
// add user intent to exact match for molecule
// lazy
//        todo check with industry wise lazy predicates
        return true;
    }
}
