package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

/**
 * Base class for match, all different matches should be extended from this one
 */
public abstract class BaseMatch<T extends IndigoRecord> extends IndigoPredicate<T> {

    private final T target;
    private final double threshold;

    public BaseMatch(T target) {
        this(target, 0.0f);
    }

    public BaseMatch(T target, double threshold) {
        this.target = target;
        this.threshold = threshold;
    }

    public T getTarget() {
        return target;
    }

    public double getThreshold() {
        return threshold;
    }

    public abstract String getFingerprintName();

    public abstract Script generateScript();

    public abstract String getMinimumShouldMatch(int length);
}
