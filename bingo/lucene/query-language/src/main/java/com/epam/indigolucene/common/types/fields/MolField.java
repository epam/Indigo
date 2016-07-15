package com.epam.indigolucene.common.types.fields;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.values.MolValue;
import com.epam.indigolucene.common.types.conditions.molconditions.MolHasSubstructureCondition;

/**
 * Created by Artem Malykh on 20.02.16.
 */
public class MolField<S> extends Field<S, IndigoObject, MolValue> {
    public MolField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    @Override
    public MolValue<S> createValue(IndigoObject from) {
        return new MolValue<>(from, this);
    }

    public final String getFingerprintFieldName() {
        return name + "_fingerprint";
    }
}
