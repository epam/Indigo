package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

public class IndigoRecord {

    private String cml;
    // Internal Elastic ID
    private String internalID = null;
    private float score;
    private List<Integer> fingerprint;
    private byte[] cmf;
    //    custom map/dict think about it as JSON
    //    object to be string?
    private final Map<String, Object> objects = new HashMap<>();

    public static class IndigoRecordBuilder {
        private final List<Consumer<IndigoRecord>> operations;

        public IndigoRecordBuilder() {
            this.operations = new ArrayList<>();
        }

        public IndigoRecordBuilder withIndigoObject(IndigoObject indigoObject) {

            withCmf(indigoObject.serialize());
            operations.add(record -> {
                List<Integer> fin = new ArrayList<>();
                String[] oneBits = indigoObject.fingerprint("sim").oneBitsList().split(" ");
                for (String oneBit : oneBits) {
                    fin.add(Integer.parseInt(oneBit));
                }
                record.fingerprint = new ArrayList<>();
                record.fingerprint.addAll(fin);
            });

            return this;
        }

        public IndigoRecordBuilder withCustomObject(String key, Object object) {
            operations.add(record -> record.objects.put(key, object));
            return this;
        }

        public IndigoRecordBuilder withFingerprint(List<Integer> fingerprint) {
            operations.add(record -> record.fingerprint = fingerprint);
            return this;
        }

        public IndigoRecordBuilder withCmf(byte[] cmf) {
            operations.add(record -> record.cmf = cmf);
            return this;
        }

        public IndigoRecordBuilder withId(String id) {
            operations.add(record -> record.internalID = id);
            return this;
        }

        public IndigoRecordBuilder withScore(float score) {
            operations.add(record -> record.score = score);
            return this;
        }


        public IndigoRecord build() {
            IndigoRecord record = new IndigoRecord();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

        public void validate(IndigoRecord record) {
            if (null == record.fingerprint) {
                //throw new Exception("Fingerprint is required field");
            }
        }
    }


    public String getInternalID() {
        return internalID;
    }

    public List<Integer> getFingerprint() {
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

    public float getScore() {
        return score;
    }

    public IndigoRecord() {

    }

    public void addCustomObject(String key, Object object) {
        this.objects.put(key, object);
    }

    public IndigoObject getIndigoObject(Indigo session) {
        return session.unserialize(getCmf());
    }


}

