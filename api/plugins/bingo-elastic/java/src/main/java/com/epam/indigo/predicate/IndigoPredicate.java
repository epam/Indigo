package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;

import java.util.function.Predicate;

public class IndigoPredicate<T extends IndigoRecord> implements Predicate<T> {


    @Override
    public boolean test(T t) {
        return false;
    }

    @Override
    public Predicate<T> and(Predicate<? super T> other) {
        return null;
    }

    @Override
    public Predicate<T> negate() {
        return null;
    }

    @Override
    public Predicate<T> or(Predicate<? super T> other) {
        return null;
    }
}
