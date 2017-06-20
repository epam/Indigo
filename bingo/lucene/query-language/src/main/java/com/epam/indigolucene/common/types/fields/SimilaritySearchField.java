package com.epam.indigolucene.common.types.fields;
/**
 * Represents interface for fields, which can have similarity condition.
 *
 * @author Artem Malykh
 * created on 2016-04-12
 */
public interface SimilaritySearchField<S, Vs, C> {
    C isSimilarTo(Vs source);


    C unsafeIsSimilarTo(String source);
}
