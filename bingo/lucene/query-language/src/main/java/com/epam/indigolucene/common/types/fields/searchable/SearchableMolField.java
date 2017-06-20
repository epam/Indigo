package com.epam.indigolucene.common.types.fields.searchable;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.molconditions.MolHasSubstructureCondition;
import com.epam.indigolucene.common.types.conditions.molconditions.MolMatchesExactlyCondition;
import com.epam.indigolucene.common.types.conditions.similarityconditions.MolSimilarTo;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.types.fields.SimilaritySearchField;
/**
 * This class is a type of "mol" field representation of Solr's schema.xml. All search methods for molecule are returned
 * from here.
 *
 * @author Artem Malykh
 * created on 2016-03-30
 */
public class SearchableMolField<S> extends MolField<S> implements SimilaritySearchField<S, IndigoObject, MolSimilarTo<S>> {
    public SearchableMolField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    public MolHasSubstructureCondition<S> hasSubstructure(IndigoObject substructure) {
        return new MolHasSubstructureCondition<>(this, substructure);
    }

    public MolHasSubstructureCondition<S> unsafeHasSubstructure(String substructure) {
        return new MolHasSubstructureCondition<>(this, substructure);
    }

    public MolMatchesExactlyCondition<S> exactMatches(IndigoObject substructure) {
        return new MolMatchesExactlyCondition<>(this, substructure);
    }

    public MolMatchesExactlyCondition<S> unsafeExactMatches(String substructure) {
        return new MolMatchesExactlyCondition<>(this, substructure);
    }

    @Override
    public MolSimilarTo<S> isSimilarTo(IndigoObject source) {
        return new MolSimilarTo<>(this, source);
    }

    @Override
    public MolSimilarTo<S> unsafeIsSimilarTo(String source) {
        return new MolSimilarTo<>(this, source);
    }
}
