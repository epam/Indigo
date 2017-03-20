package com.epam.indigolucene.common.types.conditions.reactconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.Field;

/**
 * Created by Filipp Pisarev on 20.03.17.
 */
public abstract class ReactStructureCondition<S> extends FieldCondition<S> {
    public ReactStructureCondition(Field<S, ?, ?> field) {
        super(field);
    }

    public abstract boolean doMatch(IndigoObject obj);

    public final boolean match(IndigoObject obj) {
        return this.isNegated() != doMatch(obj);
    }
}
