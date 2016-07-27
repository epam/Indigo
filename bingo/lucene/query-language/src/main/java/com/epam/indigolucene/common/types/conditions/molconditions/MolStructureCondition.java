package com.epam.indigolucene.common.types.conditions.molconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.Field;

/**
 * Created by Artem Malykh on 25.02.16.
 */
public abstract class MolStructureCondition<S> extends FieldCondition<S> {

    public MolStructureCondition(Field<S, ?, ?> field) {
        super(field);
    }

    public abstract boolean doMatch(IndigoObject obj);
    public final boolean match(IndigoObject obj) {
        return this.isNegated() != doMatch(obj);
    }
}
