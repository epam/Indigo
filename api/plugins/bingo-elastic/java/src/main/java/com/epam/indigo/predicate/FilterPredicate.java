package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.index.query.QueryBuilder;

public abstract class FilterPredicate<T extends IndigoRecord> extends IndigoPredicate<T> {

    public abstract QueryBuilder generateQuery();
}
