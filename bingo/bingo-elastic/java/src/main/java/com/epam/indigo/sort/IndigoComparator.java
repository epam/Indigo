package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;

import java.util.Comparator;


public abstract class IndigoComparator<T extends IndigoRecord> implements Comparator<T> {
    protected SortOrder sortOrder;

    public IndigoComparator(SortOrder sortOrder) {
        this.sortOrder = sortOrder;
    }

    public abstract SortBuilder toSortBuilder();

}
