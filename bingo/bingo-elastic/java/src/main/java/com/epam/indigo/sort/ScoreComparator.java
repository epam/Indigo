package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.ScoreSortBuilder;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;

public class ScoreComparator<T extends IndigoRecord> extends IndigoComparator<T> {

    public ScoreComparator() {
        super(SortOrder.DESC);
    }

    public ScoreComparator(SortOrder sortOrder) {
        super(sortOrder);
    }

    @Override
    public SortBuilder<ScoreSortBuilder> toSortBuilder() {
        return new ScoreSortBuilder().order(sortOrder);
    }

    @Override
    public int compare(final T o1, final T o2) {
        // does not expect to be called
        return 0;
    }
}
