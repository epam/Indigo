package com.epam.indigolucene.common.types.fields.searchable;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.reactconditions.ReactHasSubstructureCondition;
import com.epam.indigolucene.common.types.conditions.reactconditions.ReactMatchesExactlyCondition;
import com.epam.indigolucene.common.types.conditions.similarityconditions.ReactSimilarTo;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.types.fields.SimilaritySearchField;
/**
 * This class is a type of "react" field representation of Solr's schema.xml. All search methods for reaction are returned
 * from here.
 *
 * @author Filipp Pisarev
 * created on 2017-03-19
 */
public class SearchableReactField<S> extends ReactField<S> implements SimilaritySearchField<S, IndigoObject, ReactSimilarTo<S>> {
    public SearchableReactField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    public ReactHasSubstructureCondition<S> hasSubstructure(IndigoObject substructure) {
        return new ReactHasSubstructureCondition<>(this, substructure);
    }

    public ReactHasSubstructureCondition<S> unsafeHasSubstructure(String substructure) {
        return new ReactHasSubstructureCondition<>(this, substructure);
    }

    public ReactMatchesExactlyCondition<S> exactMatches(IndigoObject match) {
        return new ReactMatchesExactlyCondition<>(this, match);
    }

    public ReactMatchesExactlyCondition<S> unsafeExactMatches(String match) {
        return new ReactMatchesExactlyCondition<>(this, match);
    }

    @Override
    public ReactSimilarTo<S> isSimilarTo(IndigoObject source) {
        return new ReactSimilarTo<>(this, source);
    }

    @Override
    public ReactSimilarTo<S> unsafeIsSimilarTo(String source) {
        return new ReactSimilarTo<>(this, source);
    }

}
