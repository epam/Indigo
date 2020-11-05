package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.index.query.QueryBuilder;
import org.elasticsearch.index.query.QueryBuilders;

public class RangeQuery<T extends IndigoRecord> extends FilterPredicate<T> {

    private final String field;
    private final int lowerBound;
    private final int upperBound;

    public RangeQuery(String field, int lowerBound, int upperBound) {
        this.field = field;
        this.lowerBound = lowerBound;
        this.upperBound = upperBound;
    }

    @Override
    public QueryBuilder generateQuery() {
        return QueryBuilders.rangeQuery(field).gte(this.lowerBound).lte(upperBound);
    }
}
