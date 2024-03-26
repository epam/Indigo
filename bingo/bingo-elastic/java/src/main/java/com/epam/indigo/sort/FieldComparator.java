package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortBuilders;
import org.elasticsearch.search.sort.SortOrder;

import java.util.Collection;
import java.util.Collections;

public class FieldComparator<T extends IndigoRecord> extends IndigoComparator<T> {

    protected String fieldName;
    protected SortOrder sortOrder;

    public FieldComparator(final String fieldName, final SortOrder sortOrder) {
        this.fieldName = fieldName;
        this.sortOrder = sortOrder;
    }

    public String getFieldName() {
        return fieldName;
    }

    public SortOrder getSortOrder() {
        return sortOrder;
    }

    @Override
    public Collection<SortBuilder<?>> toSortBuilders() {
        return Collections.singletonList(
                SortBuilders.fieldSort(fieldName)
                        .order(sortOrder)
        );
    }

    public static <T extends IndigoRecord> Builder<T> builder() {
        return new Builder<>();
    }

    public static class Builder<T extends IndigoRecord> {
        private String fieldName;
        private SortOrder sortOrder;

        public Builder<T> withFieldName(String fieldName) {
            this.fieldName = fieldName;
            return this;
        }

        public Builder<T> withSortOrder(SortOrder sortOrder) {
            this.sortOrder = sortOrder;
            return this;
        }

        public FieldComparator<T> build() {
            return new FieldComparator<>(fieldName, sortOrder);
        }
    }
}
