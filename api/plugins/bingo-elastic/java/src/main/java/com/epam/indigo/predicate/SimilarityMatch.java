package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

public abstract class SimilarityMatch<T extends IndigoRecord> extends IndigoPredicate<T> {

    private final T target;
    private final float threshold;

    public SimilarityMatch(T target) {
        this(target, 0.0f);
    }

    public SimilarityMatch(T target, float threshold) {
        this.target = target;
        this.threshold = threshold;
    }

    public T getTarget() {
        return target;
    }

    public float getThreshold() {
        return threshold;
    }

    public abstract Script generateScript();

    public abstract String getMinimumShouldMatch(int length);
}
