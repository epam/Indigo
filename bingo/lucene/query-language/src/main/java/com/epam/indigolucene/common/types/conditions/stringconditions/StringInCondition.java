package com.epam.indigolucene.common.types.conditions.stringconditions;

import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.StringField;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.stream.Collectors;
/**
 * Represents a condition of "string in" of a SearchableStringField. Used as returning object on "in"
 * invocation from searchable string type of Solr's schema.xml class representation.
 *
 * @author Artem Malykh
 * created on 2016-04-24
 */
public class StringInCondition<S> extends FieldCondition<S> {
    public static final String OP_STRING_IN = "string_in";
    public static final String KEY_STRING_IN = "in";

    private Collection<String> strings;

    public StringInCondition(StringField<S> field, Collection<String> strings) {
        super(field);
        this.strings = strings;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        //noinspection unchecked
        obj.put(KEY_STRING_IN, strings);
    }

    @Override
    public String operationName() {
        return OP_STRING_IN;
    }

    @Override
    public List<String> getSolrFQs() {
        List<String> res = new LinkedList<>();
        Collection<String> stringsTmp;
        String operand = isNegated() ? "AND" : "OR";
        if(isNegated()) {
            stringsTmp = strings.stream().map(s -> "-" + s).collect(Collectors.toList());
        } else {
            stringsTmp = strings;
        }
        res.add(String.join(operand, stringsTmp));
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
    public StringInCondition<S> not() {
        StringInCondition<S> res = new StringInCondition<>(getField(), strings);
        res.not = !this.not;
        return res;
    }

    public static <S> Condition<S> stringStartsWithFromJson(JSONObject json) {
        Collection<String> strings = (Collection<String>) json.get(KEY_STRING_IN);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        //TODO: do full deserealization of fields from json
        StringInCondition<S> res = new StringInCondition<>(new StringField<>(fieldName, false), strings);
        res.not = (boolean) json.get(NOT);
        return res;
    }
}
