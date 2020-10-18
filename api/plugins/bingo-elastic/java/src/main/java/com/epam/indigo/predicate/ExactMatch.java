package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;

public class ExactMatch<T extends IndigoRecord> extends IndigoPredicate<T> {

    private final T target;

    public ExactMatch(T target) {
        this.target = target;
    }
}
