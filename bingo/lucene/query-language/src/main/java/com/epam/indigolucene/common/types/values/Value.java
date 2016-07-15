package com.epam.indigolucene.common.types.values;

import com.epam.indigolucene.common.types.fields.Field;

import java.util.Map;

/**
 * Created by enny on 28.03.16.
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
