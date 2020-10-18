package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.index.query.QueryBuilder;
import org.elasticsearch.index.query.QueryBuilders;

public class WildcardQuery<T extends IndigoRecord> extends FilterPredicate<T> {

    private final String field;
    private final String textToSearch;

    public WildcardQuery(String field, String textToSearch) {
        this.field = field;
        this.textToSearch = textToSearch;
    }

    @Override
    public QueryBuilder generateQuery() {
        return QueryBuilders.wildcardQuery(field, textToSearch);
    }
}
