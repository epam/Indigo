package com.epam.indigo.predicate;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.IndigoRecord;

import java.util.function.Predicate;

public class IndigoPredicate<T extends IndigoRecord> implements Predicate<T> {


    @Override
    public boolean test(T t) {
        return false;
    }

    @Override
    public Predicate<T> and(Predicate<? super T> other) {
        throw new BingoElasticException("and() operation on this predicate isn't implemented");
    }

    @Override
    public Predicate<T> negate() {
        throw new BingoElasticException("negate() operation on this predicate isn't implemented");
    }

    @Override
    public Predicate<T> or(Predicate<? super T> other) {
        throw new BingoElasticException("or() operation on this predicate isn't implemented");
    }
}
