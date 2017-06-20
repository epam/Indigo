package com.epam.indigolucene.common.types.conditions;

import com.epam.indigolucene.common.types.fields.Field;
import org.json.simple.JSONObject;

/**
 * This class encapsulates field representation used in queries to solr.
 *
 * @author Artem Malykh
 * created on 2016-02-20
 */
public abstract class FieldCondition<S> extends AbstractCondition<S> {
    public static final String FIELD_NAME = "fieldName";
    private Field<S, ?, ?> field;

    public FieldCondition(Field<S, ?, ?> field) {
        this.field = field;
    }

    public String getFieldName() {
        return field.getName();
    }

    public Field<S, ?, ?> getField() {
        return field;
    }

    @Override
    protected final void addToJson(JSONObject obj) {
        //noinspection unchecked
        obj.put(FIELD_NAME, getFieldName());
        //noinspection unchecked
        obj.put(NOT, not);
        addFieldConditionDataToJson(obj);
    }

    protected abstract void addFieldConditionDataToJson(JSONObject obj);
}
