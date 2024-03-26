package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.SortBuilder;

import java.util.ArrayList;
import java.util.Collection;
import java.util.stream.Collectors;

public class CombinedComparator<T extends IndigoRecord> extends IndigoComparator<T> {

    protected Collection<IndigoComparator<T>> comparators;

    @Override
    public Collection<SortBuilder<?>> toSortBuilders() {
        return comparators.stream()
                .map(IndigoComparator::toSortBuilders)
                .flatMap(Collection::stream)
                .collect(Collectors.toList());
    }

    public static <T extends IndigoRecord> Builder<T> builder() {
        return new Builder<>();
    }

    public static class Builder<T extends IndigoRecord> {
        private Collection<IndigoComparator<T>> comparators;

        public Builder<T> withComparators(Collection<IndigoComparator<T>> comparators) {
            this.comparators = comparators;
            return this;
        }

        public Builder<T> withComparator(IndigoComparator<T> comparator) {
            if (this.comparators == null)
                this.comparators = new ArrayList<>();
            this.comparators.add(comparator);
            return this;
        }

        public CombinedComparator<T> build() {
            CombinedComparator<T> combinedComparator = new CombinedComparator<>();
            combinedComparator.comparators = comparators;
            return combinedComparator;
        }
    }

}
