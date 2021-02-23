package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.fields.Field;
import com.epam.indigo.model.fields.FieldNotFoundException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

public class IndigoRecord {

    //    custom map/dict think about it as JSON
    //    object to be string?
    private final Map<String, Object> objects = new HashMap<>();
    // Internal Elastic ID
    private String internalID = null;
    private float score;
    // todo: rename? and add ability to extend?
    private List<Integer> simFingerprint;
    private List<Integer> subFingerprint;
    //    TODO add tau fingerprint, add support for other fingerprints
    private byte[] cmf;
    private String name;

    public IndigoRecord() {

    }

    public String getInternalID() {
        return internalID;
    }

    public List<Integer> getSimFingerprint() {
        return simFingerprint;
    }

    public List<Integer> getSubFingerprint() {
        return subFingerprint;
    }

    public byte[] getCmf() {
        return cmf;
    }

    public String getName() {
        return name;
    }

    public Map<String, Object> getObjects() {
        return objects;
    }

    public float getScore() {
        return score;
    }

    public void addCustomObject(String key, Object object) {
        this.objects.put(key, object);
    }

    public IndigoObject getIndigoObject(Indigo session) {
        return session.unserialize(getCmf());
    }

    public Field getField(String field) throws FieldNotFoundException {
        Object value = objects.get(field);
        if (null == value) {
            throw new FieldNotFoundException();
        }
        return new Field(value);
    }

    public static class IndigoRecordBuilder {
        private final List<Consumer<IndigoRecord>> operations;

        public IndigoRecordBuilder() {
            this.operations = new ArrayList<>();
        }

        public IndigoRecordBuilder withIndigoObject(IndigoObject indigoObject) {
            withCmf(indigoObject.serialize());
            withName(indigoObject.name());
            operations.add(record -> {
                List<Integer> fin = new ArrayList<>();
                String simBitList = indigoObject.fingerprint("sim").oneBitsList();
                String subBitList = indigoObject.fingerprint("sub").oneBitsList();
                String[] oneBits = simBitList.split(" ");

                if (simBitList.length() == 0 || subBitList.length() == 0) {
                    throw new BingoElasticException("Building IndigoRecords from empty IndigoObject is not supported");
                }

                for (String oneBit : oneBits) {
                    fin.add(Integer.parseInt(oneBit));
                }
                record.simFingerprint = new ArrayList<>();
                record.simFingerprint.addAll(fin);
                fin.clear();
                oneBits = subBitList.split(" ");
                for (String oneBit : oneBits) {
                    fin.add(Integer.parseInt(oneBit));
                }
                record.subFingerprint = new ArrayList<>();
                record.subFingerprint.addAll(fin);
            });

            return this;
        }

        public IndigoRecordBuilder withCustomObject(String key, Object object) {
            operations.add(record -> record.objects.put(key, object));
            return this;
        }

        public IndigoRecordBuilder withSimFingerprint(List<Integer> simFingerprint) {
            operations.add(record -> record.simFingerprint = simFingerprint);
            return this;
        }

        public IndigoRecordBuilder withSubFingerprint(List<Integer> subFingerprint) {
            operations.add(record -> record.subFingerprint = subFingerprint);
            return this;
        }

        public IndigoRecordBuilder withCmf(byte[] cmf) {
            operations.add(record -> record.cmf = cmf);
            return this;
        }

        public IndigoRecordBuilder withName(String name) {
            operations.add(record -> record.name = name);
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


        public IndigoRecord build() throws BingoElasticException {
            IndigoRecord record = new IndigoRecord();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

        public void validate(IndigoRecord record) throws BingoElasticException {
            if (record.internalID == null) {
                if (null == record.simFingerprint) {
                    throw new BingoElasticException("Fingerprint is required field");
                }
                if (null == record.subFingerprint) {
                    throw new BingoElasticException("Fingerprint is required field");
                }
            }
        }
    }

}

