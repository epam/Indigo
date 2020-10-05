package com.epam.indigo.model;

import com.epam.indigo.IndigoObject;

import java.util.ArrayList;
import java.util.List;

public class IndigoRecord {

    // Internal Elastic ID
    String _id = null;

    final List<Short> fingerprint;
    byte[] cmf;

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

