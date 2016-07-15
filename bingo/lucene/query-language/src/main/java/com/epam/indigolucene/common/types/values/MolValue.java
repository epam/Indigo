package com.epam.indigolucene.common.types.values;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.utils.Utils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by Artem Malykh on 28.03.16.
 */
public class MolValue<S> extends Value<MolField<S>> {
    IndigoObject obj;

    public MolValue(IndigoObject from, MolField<S> f) {
        super(f);
        obj = from;
    }

    @Override
    public Map<String, Object> toMap() {
        Map<String, Object> res = new HashMap<>();
        obj.aromatize();
        res.put(field.getName(), obj.serialize());
        IndigoObject fingerprint = obj.fingerprint();
        res.put(field.getFingerprintFieldName(), Utils.fingerprintToBitNums(fingerprint));
        return res;
    }
}
