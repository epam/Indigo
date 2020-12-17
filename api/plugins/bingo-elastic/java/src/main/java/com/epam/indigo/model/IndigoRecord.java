package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.fields.Field;
import com.epam.indigo.model.fields.FieldNotFoundException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

class SkipErrorsHandler implements ErrorHandler {
    @Override
    public void handle(IndigoException error) {

    }
}

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

    // Skip errors when IndigoRecord cannot be created
    private Boolean skipErrors = false;

    // Error handler called when IndigoRecord cannot be created
    private ErrorHandler errorHandler;

    public IndigoRecord() {}

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
        return session.deserialize(getCmf());
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
                record.subFingerprint = addFingerprint(indigoObject, "sub");
                record.simFingerprint = addFingerprint(indigoObject, "sim");
            });

            return this;
        }

        private List<Integer> addFingerprint(IndigoObject indigoObject, String fingerprintType) {
            List<Integer> result = new ArrayList<>();
            String bitList = indigoObject.fingerprint(fingerprintType).oneBitsList();
            if (bitList.isEmpty()) {
                return result;
            }
            String[] oneBits = bitList.split(" ");
            for (String oneBit : oneBits) {
                result.add(Integer.parseInt(oneBit));
            }
            return result;
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

        public IndigoRecordBuilder withErrorHandler(ErrorHandler errorHandler) {
            operations.add(record -> record.errorHandler = errorHandler);
            return this;
        }

        public IndigoRecordBuilder withSkipErrors(Boolean skipErrors) {
            operations.add(record -> record.skipErrors = skipErrors);
            return this;
        }


        public IndigoRecord build() throws BingoElasticException {
            IndigoRecord record = new IndigoRecord();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

        public void validate(IndigoRecord record) throws BingoElasticException {
            if (record.cmf.length == 0)
                throw new BingoElasticException("Creation of IndgioRecord from empty IndigoObject isn't supported");
        }
    }

}

