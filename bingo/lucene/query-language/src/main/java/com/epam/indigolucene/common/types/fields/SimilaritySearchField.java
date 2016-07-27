package com.epam.indigolucene.common.types.fields;

/**
 * Created by Artem_Malykh on 12.04.16.
 */
public interface SimilaritySearchField<S, Vs, C> {
    C isSimilarTo(Vs source);


    C unsafeIsSimilarTo(String source);
}
