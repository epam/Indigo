package com.epam.indigo.model;

import com.epam.indigo.IndigoObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class IndigoRecord {

    // Internal Elastic ID
    String _id = null;

    final List<Short> fingerprint;
    byte[] cmf;
//    custom map/dict think about it as JSON
//    object to be string?
    Map<String, Object> objects;

    public String get_id() {
        return _id;
    }

    public void set_id(String _id) {
        this._id = _id;
    }

    public List<Short> getFingerprint() {
        return fingerprint;
    }

    public byte[] getCmf() {
        return cmf;
    }

    public void setCmf(byte[] cmf) {
        this.cmf = cmf;
    }

    public Map<String, Object> getObjects() {
        return objects;
    }

    public IndigoRecord() {
        this.fingerprint = null;
    }


    public IndigoRecord(IndigoObject indObject) {
        this.fingerprint = new ArrayList<>();

        String[] oneBits = indObject.fingerprint("sim").oneBitsList().split(" ");
        for (String oneBit : oneBits) {
            fingerprint.add(Short.parseShort(oneBit));
        }
    }


}

