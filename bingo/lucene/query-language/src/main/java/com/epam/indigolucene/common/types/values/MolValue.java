package com.epam.indigolucene.common.types.values;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.utils.Utils;

import java.util.HashMap;
import java.util.Map;
/**
 * MolValue class maintain a value of a molecule along with "toMap" method, which
 * returns a map structure of field name and fingerprint num. representation
 *
 * @author Artem Malykh
 * created on 2016-03-28
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
