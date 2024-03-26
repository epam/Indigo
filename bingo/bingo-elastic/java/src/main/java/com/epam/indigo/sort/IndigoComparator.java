package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.SortBuilder;

import java.util.Collection;
import java.util.Comparator;


public abstract class IndigoComparator<T extends IndigoRecord> implements Comparator<T> {

    public abstract Collection<SortBuilder<?>> toSortBuilders();

    @Override
    public int compare(final T o1, final T o2) {
        // does not expect to be called
        return 0;
    }

}
