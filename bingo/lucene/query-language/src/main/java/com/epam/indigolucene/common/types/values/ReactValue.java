package com.epam.indigolucene.common.types.values;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.utils.Utils;

import java.util.HashMap;
import java.util.Map;
/**
 * ReactValue class maintain a value of a reaction along with "toMap" method, which
 * returns a map structure of field name and fingerprint num. representation
 *
 * @author Filipp Pisarev
 * created on 2017-03-19
 */
public class ReactValue<S> extends Value<ReactField<S>>{
    IndigoObject obj;

    public ReactValue(IndigoObject from,ReactField<S> f) {
        super(f);
        obj = from;
    }

    @Override
    public Map<String, Object> toMap() {
        Map<String, Object> res = new HashMap<>();
        obj.aromatize();
        res.put(field.getName(), obj.serialize());
        IndigoObject fingerprint = obj.fingerprint();
        res.put(field.getFingerPrintFieldName(), Utils.fingerprintToBitNums(fingerprint));
        return res;
    }
}
