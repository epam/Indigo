package com.epam.indigolucene.common.types.fields.searchable;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.reactconditions.ReactHasSubstructureCondition;
import com.epam.indigolucene.common.types.fields.ReactField;

/**
 * Created by Filipp Pisarev on 19.03.17.
 */
public class SearchableReactField<S> extends ReactField<S> {
    public SearchableReactField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    public ReactHasSubstructureCondition<S> hasSubstructure(IndigoObject substructure) {
        return new ReactHasSubstructureCondition<>(this, substructure);
    }

    public ReactHasSubstructureCondition<S> unsafeHasSubstructure(String substructure) {
        return new ReactHasSubstructureCondition<>(this, substructure);
    }
}
