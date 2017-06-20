package com.epam.indigolucene.common.types.values;

import com.epam.indigolucene.common.types.fields.Field;

import java.util.Map;

/**
 * A class which is a basis for other value classes, e.g. "ReactValue".
 *
 * @author enny
 * created on 2016-03-28
 */
public abstract class Value<F extends Field> {
    protected F field;

    public Value(F field) {
        this.field = field;
    }

    public abstract Map<String, Object> toMap();

    public F getField() {
        return field;
    }
}
