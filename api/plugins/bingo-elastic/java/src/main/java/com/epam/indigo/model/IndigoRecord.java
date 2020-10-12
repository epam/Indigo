package com.epam.indigo.model;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

public class IndigoRecord {

    private String cml;
    // Internal Elastic ID
    private String internalID = null;

    private short[] fingerprint;
    private byte[] cmf;
    //    custom map/dict think about it as JSON
//    object to be string?
    Map<String, Object> objects;

    public static class IndigoRecordBuilder {
        private List<Consumer<IndigoRecord>> operations;

        public IndigoRecordBuilder() {
            this.operations = new ArrayList<>();
        }

        public IndigoRecordBuilder withCustomObject(String key, Object object) {
            operations.add(repo -> repo.objects.put(key, object));
            return this;
        }
    }


    public String getInternalID() {
        return internalID;
    }

    public short[] getFingerprint() {
        return fingerprint;
    }

    public byte[] getCmf() {
        return cmf;
    }

    public String getCml() {
        return cml;
    }

    public Map<String, Object> getObjects() {
        return objects;
    }

//    public IndigoRecord(IndigoObject indObject) {
//        List<Short> fin = new ArrayList<>();
//
//        String[] oneBits = indObject.fingerprint("sim").oneBitsList().split(" ");
//        this.objects = new HashMap<>();
//        this.cml = indObject.cml(); // todo: remove?
//        this.cmf = indObject.serialize();
//        for (String oneBit : oneBits) {
//            fin.add(Short.parseShort(oneBit));
//        }
//        this.fingerprint = new short[fin.size()];
//        int i = 0;
//        for (Short bit : fin) {
//            this.fingerprint[i++] = bit;
//        }
//    }
//
//    public void addCustomObject(String key, Object object) {
//        this.objects.put(key, object);
//    }


}

