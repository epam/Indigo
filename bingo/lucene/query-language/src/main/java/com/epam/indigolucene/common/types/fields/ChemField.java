package com.epam.indigolucene.common.types.fields;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.values.ChemValue;

public class ChemField<S> extends Field<S, IndigoObject, ChemValue> {
    public ChemField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    @Override
    public ChemValue<S> createValue(IndigoObject from) {
        return new ChemValue<>(from, this);
    }

    public final String getFingerPrintFieldName() {
        return name + "_fingerprint";
    }
}