package com.epam.indigolucene.common.types.fields;

import com.epam.indigolucene.common.types.values.StringValue;

import java.util.List;

/**
 * Created by Artem Malykh on 20.02.16.
 */
public class StringField<S> extends Field<S, String, StringValue> implements MultipliableField<String, StringValue> {

    public StringField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    @Override
    public StringValue<S> createValue(String from) {
        return new StringValue<>(from, this);
    }

    @Override
    public StringValue createValue(List<String> vals) {
        StringValue<S> res = new StringValue<S>(this);
        res.setMultipleValues(vals);
        return res;
    }
}
