package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortBuilders;
import org.elasticsearch.search.sort.SortOrder;

import java.util.Collection;
import java.util.Collections;

public class ScoreComparator<T extends IndigoRecord> extends IndigoComparator<T> {

    protected SortOrder sortOrder;

    public ScoreComparator() {
        this(SortOrder.DESC);
    }

    public ScoreComparator(SortOrder sortOrder) {
        this.sortOrder = sortOrder;
    }

    public SortOrder getSortOrder() {
        return sortOrder;
    }

    @Override
    public Collection<SortBuilder<?>> toSortBuilders() {
        return Collections.singletonList(SortBuilders.scoreSort().order(sortOrder));
    }

    public static <T extends IndigoRecord> Builder<T> builder() {
        return new Builder<>();
    }

    public static class Builder<T extends IndigoRecord> {
        private SortOrder sortOrder;

        public Builder<T> withSortOrder(SortOrder sortOrder) {
            this.sortOrder = sortOrder;
            return this;
        }

        public ScoreComparator<T> build() {
            return new ScoreComparator<>(sortOrder);
        }
    }

}
