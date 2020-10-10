package com.epam.indigo.model;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.Indigo;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class IndigoRecord {

    private String cml;
    // Internal Elastic ID
    private String _id = null;

    private final short[] fingerprint;
    private byte[] cmf;
    //    custom map/dict think about it as JSON
//    object to be string?
    Map<String, Object> objects;

    public IndigoRecord(String id, Map<String, Object> sourceAsMap) {
        this._id = id;
        this.objects = sourceAsMap;
        this.fingerprint = null;
    }

    public static IndigoRecord fromMap(String _id, Map<String, Object> sourceAsMap) {
        IndigoRecord indigoRecord = new IndigoRecord(_id, sourceAsMap);
        return indigoRecord;
    }

    public String get_id() {
        return _id;
    }

    public void set_id(String _id) {
        this._id = _id;
    }

    public short[] getFingerprint() {
        return fingerprint;
    }

    public byte[] getCmf() {
        return cmf;
    }

    public void setCmf(byte[] cmf) {
        this.cmf = cmf;
    }

    public String getCml() {
        return cml;
    }

    public Map<String, Object> getObjects() {
        return objects;
    }

    public IndigoRecord(IndigoObject indObject) {
        List<Short> fin = new ArrayList<>();

        String[] oneBits = indObject.fingerprint("sim").oneBitsList().split(" ");
        this.objects = new HashMap<>();
        this.cml = indObject.cml();
        for (String oneBit : oneBits) {
            fin.add(Short.parseShort(oneBit));
        }
        this.fingerprint = new short[fin.size()];
        int i = 0;
        for (Short bit : fin) {
            this.fingerprint[i++] = bit;
        }
    }

    public static IndigoRecord loadFromFile(String molfile) {
        Indigo indigo = new Indigo();
        IndigoRecord curObject = new IndigoRecord(
                indigo.loadMoleculeFromFile(molfile)
        );
        return curObject;
    }

    public static List<IndigoRecord> loadFromSdf(String molfile) {
        return loadFromSdf(molfile, true);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile, Boolean skipErrors) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> recordList = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSDFile(sdfFile)) {
            try {
                recordList.add(new IndigoRecord(comp));
            } catch (Exception e) {
                // todo: change to indigo exception
                if (!skipErrors) {
                    throw e;
                }
            }
        }
        return recordList;
    }

    public void addCustomObject(String key, Object object) {
        this.objects.put(key, object);
    }


}

