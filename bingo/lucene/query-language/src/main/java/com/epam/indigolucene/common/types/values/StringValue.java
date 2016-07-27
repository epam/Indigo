package com.epam.indigolucene.common.types.values;

import com.epam.indigolucene.common.types.fields.StringField;

import java.util.*;

/**
 * Created by enny on 28.03.16.
 */
public class StringValue<S> extends Value<StringField<S>> implements MultipliableValue<String>{
    private List<String> val;

    public StringValue(String from, StringField<S> f) {
        this(f);
        val.add(from);
    }

    public StringValue(StringField<S> f) {
        super(f);
        val = new LinkedList<>();
    }

    @Override
    public Map<String, Object> toMap() {
        if (getField().isMultiple()) {
            return toMapMultiple();
        } else {
            Map<String, Object> res = new HashMap<>();
            res.put(field.getName(), val.get(0));
            return res;
        }
    }

    @Override
    public void setMultipleValues(List<String> vals) {
        val.clear();
        val.addAll(vals);
    }

    @Override
    public Map<String, Object> toMapMultiple() {
        Map<String, Object> res = new HashMap<>();
        res.put(field.getName(), val);
        return res;
    }
}
