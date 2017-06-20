package com.epam.indigolucene.common.types.conditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.Field;
/**
 * ChemStructureCondition class provides a methods for chemical structure comparison.
 *
 * @author Artem Malykh
 * created on 2016-02-25
 */
public abstract class ChemStructureCondition<S> extends FieldCondition<S> {

    public ChemStructureCondition(Field<S, ?, ?> field) {
        super(field);
    }

    public abstract boolean doMatch(IndigoObject obj);
    public final boolean match(IndigoObject obj) {
        return this.isNegated() != doMatch(obj);
    }
}
