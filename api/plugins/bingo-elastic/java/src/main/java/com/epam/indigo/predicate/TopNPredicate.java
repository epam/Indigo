package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;

// TODO Replace with proper usage on limit
@Deprecated()
public class TopNPredicate<T extends IndigoRecord> extends IndigoPredicate<T> {

    private final int topN;

    public int getTopN() {
        return topN;
    }

    public TopNPredicate(int topN) {
        this.topN = topN;
    }
}
