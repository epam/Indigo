package com.epam.indigolucene.common.types.values;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.fields.ChemField;
import com.epam.indigolucene.common.utils.Utils;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by Filipp Pisarev on 19.03.17.
 */
public class ChemValue<S> extends Value<ChemField<S>>{
    IndigoObject obj;

    public ChemValue(IndigoObject from, ChemField<S> f) {
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
