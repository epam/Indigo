package com.epam.indigo.model;

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

        public IndigoRecordBuilder withIndigoObject(IndigoObject indigoObject) {

            this.withCmf(indigoObject.serialize());
            operations.add(record -> {
                List<Short> fin = new ArrayList<>();
                String[] oneBits = indigoObject.fingerprint("sim").oneBitsList().split(" ");
                for (String oneBit : oneBits) {
                    fin.add(Short.parseShort(oneBit));
                }
                record.fingerprint = new short[fin.size()];
                int i = 0;
                for (Short bit : fin) {
                    record.fingerprint[i++] = bit;
                }
            });

            return this;
        }

        public IndigoRecordBuilder withCustomObject(String key, Object object) {
            operations.add(record -> record.objects.put(key, object));
            return this;
        }

        public IndigoRecordBuilder withFingerprint(short[] fingerprint) {
            operations.add(record -> record.fingerprint = fingerprint);
            return this;
        }

        public IndigoRecordBuilder withCmf(byte[] cmf) {
            operations.add(record -> record.cmf = cmf);
            return this;
        }


        public IndigoRecord build() throws Exception {
            IndigoRecord record = new IndigoRecord();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

        public void validate(IndigoRecord record) throws Exception {
            if (null == record.fingerprint) {
                throw new Exception("Fingerprint is required field");
            }
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

        public IndigoRecord() {

        }

        public IndigoRecord(IndigoObject indObject) {
            // TODO: remove
            List<Short> fin = new ArrayList<>();

            String[] oneBits = indObject.fingerprint("sim").oneBitsList().split(" ");
            this.objects = new HashMap<>();
            this.cmf = indObject.serialize();
            for (String oneBit : oneBits) {
                fin.add(Short.parseShort(oneBit));
            }
            this.fingerprint = new short[fin.size()];
            int i = 0;
            for (Short bit : fin) {
                this.fingerprint[i++] = bit;
            }
        }

        public void addCustomObject(String key, Object object) {
            this.objects.put(key, object);
        }

    }

