package com.epam.indigolucene.common.types.values;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.utils.Utils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by Filipp Pisarev on 19.03.17.
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
