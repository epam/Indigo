package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord.IndigoRecordBuilder;

import java.util.*;


/**
 * Helpers class, that have ability to create {@link com.epam.indigo.model.IndigoRecord} from popular formats like SDF, MOL, Smiles, etc
 */
public class Helpers {

    protected static void loadOrThrow(List<IndigoRecord> acc, IndigoObject comp, boolean skipErrors) {
        try {
            acc.add(FromIndigoObject.build(comp));
        } catch (Exception e) {
            if (!skipErrors) {
                throw e;
            }
        }
    }

    protected static IndigoRecord load(IndigoObject comp) {
        return FromIndigoObject.build(comp);
    }

    public static IndigoRecord loadFromFile(String molFile) {
        Indigo indigo = new Indigo();
        return FromIndigoObject.build(indigo.loadMoleculeFromFile(molFile));
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile) throws Exception {
        return loadFromSdf(sdfFile, true);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile, boolean skipErrors) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSDFile(sdfFile)) {
            loadOrThrow(acc, comp, skipErrors);
        }
        return acc;
    }

    public static IndigoRecord loadFromSmiles(String smiles) {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule(smiles);
        return FromIndigoObject.build(indigoObject);
    }

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile) throws Exception {
        return loadFromSmilesFile(smilesFile, true);
    }

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile, boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSmilesFile(smilesFile)) {
            loadOrThrow(acc, comp, skipErrors);
        }
        return acc;
    }

    public static List<IndigoRecord> loadFromCmlFile(String cmlFile) throws Exception {
        return loadFromCmlFile(cmlFile, true);
    }

    public static List<IndigoRecord> loadFromCmlFile(String cmlFile, boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateCMLFile(cmlFile)) {
            loadOrThrow(acc, comp, skipErrors);
        }
        return acc;
    }

    public static IndigoRecord fromElastic(String id, Map<String, Object> source, float score) throws BingoElasticException {
        IndigoRecordBuilder indigoRecordBuilder = new IndigoRecordBuilder();
        for (Map.Entry<String, Object> entry : source.entrySet()) {
            if (entry.getKey().equals(NamingConstants.SIM_FINGERPRINT)) {
                indigoRecordBuilder.withSimFingerprint((List<Integer>) ((List<Object>) entry.getValue()).get(0));
            } else if (entry.getKey().equals(NamingConstants.SUB_FINGERPRINT)) {
                indigoRecordBuilder.withSubFingerprint((List<Integer>) ((List<Object>) entry.getValue()).get(0));
            } else {
                indigoRecordBuilder.withCustomObject(entry.getKey(), entry.getValue());
            }
        }
        indigoRecordBuilder.withScore(score);
        indigoRecordBuilder.withId(id);
        byte[] cmf = Base64.getDecoder().decode((String) source.get(NamingConstants.CMF));
        indigoRecordBuilder.withCmf(cmf);
        indigoRecordBuilder.withName((String) source.get(NamingConstants.NAME));
        return indigoRecordBuilder.build();
    }

    protected static Iterable<IndigoRecord> iterateIndigoObject(IndigoObject indigoObject) {
        return () -> new Iterator<IndigoRecord>() {
            @Override
            public boolean hasNext() {
                return indigoObject.hasNext();
            }

            @Override
            public IndigoRecord next() {
                return load(indigoObject.next());
            }
        };
    }

    public static Iterable<IndigoRecord> iterateSdf(String sdfFile) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSDFile(sdfFile));
    }

    public static Iterable<IndigoRecord> iterateSmiles(String smilesFile) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSmilesFile(smilesFile));
    }

    public static Iterable<IndigoRecord> iterateCml(String cmlFile) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateCMLFile(cmlFile));
    }

}
