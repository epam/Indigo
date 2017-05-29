package com.epam.indigolucene.common.types.fields.searchable;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.chemconditions.ChemHasSubstructureCondition;
import com.epam.indigolucene.common.types.conditions.chemconditions.ChemMatchesExactlyCondition;
import com.epam.indigolucene.common.types.conditions.similarityconditions.ChemSimilarTo;
import com.epam.indigolucene.common.types.fields.ChemField;
import com.epam.indigolucene.common.types.fields.SimilaritySearchField;

/**
 * Created by Artem Malykh on 30.03.16.
 */
public class SearchableChemField<S> extends ChemField<S> implements SimilaritySearchField<S, IndigoObject, ChemSimilarTo<S>> {
    public SearchableChemField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    public ChemHasSubstructureCondition<S> hasSubstructure(IndigoObject substructure) {
        return new ChemHasSubstructureCondition<>(this, substructure);
    }

    public ChemHasSubstructureCondition<S> unsafeHasSubstructure(String substructure) {
        return new ChemHasSubstructureCondition<>(this, substructure);
    }

    public ChemMatchesExactlyCondition<S> exactMatches(IndigoObject substructure) {
        return new ChemMatchesExactlyCondition<>(this, substructure);
    }

    public ChemMatchesExactlyCondition<S> unsafeExactMatches(String substructure) {
        return new ChemMatchesExactlyCondition<>(this, substructure);
    }

    @Override
    public ChemSimilarTo<S> isSimilarTo(IndigoObject source) {
        return new ChemSimilarTo<>(this, source);
    }

    @Override
    public ChemSimilarTo<S> unsafeIsSimilarTo(String source) {
        return new ChemSimilarTo<>(this, source);
    }
}
