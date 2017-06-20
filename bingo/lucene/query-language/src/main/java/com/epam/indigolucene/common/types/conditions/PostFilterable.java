package com.epam.indigolucene.common.types.conditions;

import java.util.Map;
import java.util.function.Predicate;
/**
 * This field encapsulate conditions which are based on some value of type T
 * Example: MolSimilarTo is based on similarity score.
 * Results of such queries can be further postfiltered on client
 *
 * @author Artem Malykh
 * created on 2016-04-19
 */
public interface PostFilterable<S, T> extends Condition<S> {
    T getValue(Map<String, Object> res);


    default Condition<S> addPostFilter(Predicate<T> postFilter) {
        getPostFilters().add(map -> postFilter.test(getValue(map)));
        return this;
    }
}
