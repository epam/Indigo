package com.epam.indigolucene.common.types.conditions.stringconditions;

import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.StringField;
import org.json.simple.JSONObject;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by Artem Malykh on 24.02.16.
 * This class represents condition for filtering strings starting from specified string.
 */
public class StringStartsWithCondition<S> extends FieldCondition<S> {
    public static final String OP_STARTS_WITH = "startsWith";
    public static final String KEY_STRING_STARTER = "starter";

    private String starter;

    public StringStartsWithCondition(StringField<S> field, String starter) {
        super(field);
        this.starter = starter;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        //noinspection unchecked
        obj.put(KEY_STRING_STARTER, starter);
    }

    @Override
    public String operationName() {
        return OP_STARTS_WITH;
    }

    @Override
    public List<String> getSolrFQs() {
        List<String> res = new LinkedList<>();
        res.add(getFieldName() + ":" + starter + "*");
        return  res;
    }

    @Override
    public String getSolrQ() {
        return "";
    }

    @Override
    public StringField<S> getField() {
        return (StringField<S>) super.getField();
    }

    @Override
    public StringStartsWithCondition<S> not() {
        StringStartsWithCondition<S> res = new StringStartsWithCondition<>(getField(), starter);
        res.not = !this.not;
        return res;
    }

    public static <S> Condition<S> stringStartsWithFromJson(JSONObject json) {
        String starter =      (String) json.get(KEY_STRING_STARTER);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        //TODO: do full deserealization of fields from json
        StringStartsWithCondition<S> res = new StringStartsWithCondition<>(new StringField<>(fieldName, false), starter);
        res.not = (boolean) json.get(NOT);
        return res;
    }
}
