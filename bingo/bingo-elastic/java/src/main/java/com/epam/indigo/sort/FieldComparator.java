package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.FieldSortBuilder;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;

public class FieldComparator<T extends IndigoRecord> extends IndigoComparator<T> {

    protected String fieldName;

    public FieldComparator(final String fieldName, final SortOrder sortOrder) {
        super(sortOrder);
        this.fieldName = fieldName;
    }

    @Override
    public int compare(final T o1, final T o2) {
        // does not expect to be called
        return 0;
    }

    @Override
    public SortBuilder<FieldSortBuilder> toSortBuilder() {
        return new FieldSortBuilder(this.fieldName).order(this.sortOrder);
    }
}
