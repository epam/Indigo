package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.index.query.QueryBuilder;
import org.elasticsearch.index.query.QueryBuilders;

// TODO proper name for
public class KeywordQuery<T extends IndigoRecord> extends FilterPredicate<T> {

    private final String field;
    private final String text;

    public KeywordQuery(String field, String text) {
        this.field = field;
        this.text = text;
    }

    @Override
    public QueryBuilder generateQuery() {
        return QueryBuilders.matchQuery(field, text).boost(0.0f);
    }
}
