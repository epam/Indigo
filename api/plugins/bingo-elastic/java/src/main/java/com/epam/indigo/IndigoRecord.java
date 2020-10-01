package com.epam.indigo;

import java.util.ArrayList;
import java.util.List;

public class IndigoRecord {

    // Internal Elastic ID
    String _id = null;

    final List<Short> fingerprint;
    byte[] cmf;


    public IndigoRecord(IndigoObject indObject) {
        this.fingerprint = new ArrayList<>();

        String[] oneBits = indObject.fingerprint("sim").oneBitsList().split(" ");
        for (String oneBit : oneBits) {
            fingerprint.add(Short.parseShort(oneBit));
        }
    }


}

