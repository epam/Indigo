package com.epam.indigolucene.common.types.fields;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.values.ReactValue;
/**
 * "react" field representation of Solr's schema.xml.
 *
 * @author Filipp Pisarev
 * created on 2017-03-20
 */
public class ReactField<S> extends Field<S, IndigoObject, ReactValue> {
    public ReactField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    @Override
    public ReactValue<S> createValue(IndigoObject from) {
        return new ReactValue<>(from, this);
    }

    public final String getFingerPrintFieldName() {
        return name + "_fingerprint";
    }
}