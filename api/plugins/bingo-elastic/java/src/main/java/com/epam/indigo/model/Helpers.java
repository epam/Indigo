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

    protected static IndigoRecord load(IndigoObject comp) {
        return load(comp, error -> {});
    }

    protected static IndigoRecord load(IndigoObject comp, ErrorHandler errorHandler) {
        return FromIndigoObject.build(comp, errorHandler);
    }

    public static IndigoRecord loadFromFile(String molFile) {
        Indigo indigo = new Indigo();
        return FromIndigoObject.build(indigo.loadMoleculeFromFile(molFile));
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateSdf(String)} iterateSdf} instead
     * @param sdfFile
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromSdf(String sdfFile) {
        return loadFromSdf(sdfFile, error -> {});
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateSdf(String, ErrorHandler)} iterateSdf} instead
     * @param sdfFile
     * @param errorHandler
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromSdf(String sdfFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSDFile(sdfFile)) {
            acc.add(FromIndigoObject.build(comp, errorHandler));
        }
        return acc;
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateSmiles(String)} iterateSdf} instead
     * @param smilesFile
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile) {
        return loadFromSmilesFile(smilesFile, error -> {});
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateSmiles(String, ErrorHandler)} iterateSdf} instead
     * @param smilesFile
     * @param errorHandler
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSmilesFile(smilesFile)) {
            acc.add(FromIndigoObject.build(comp, errorHandler));
        }
        return acc;
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateCml(String)} instead
     * @param cmlFile
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromCmlFile(String cmlFile) {
        return loadFromCmlFile(cmlFile, error -> {});
    }

    /**
     * @deprecated
     * This method will be removed in next releases, use {@link com.epam.indigo.model.Helpers#iterateCml(String, ErrorHandler)} instead
     * @param cmlFile
     * @param errorHandler
     * @return
     */
    @Deprecated
    public static List<IndigoRecord> loadFromCmlFile(String cmlFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> acc = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateCMLFile(cmlFile)) {
            acc.add(FromIndigoObject.build(comp, errorHandler));
        }
        return acc;
    }


    public static IndigoRecord loadFromSmiles(String smiles) {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule(smiles);
        return FromIndigoObject.build(indigoObject);
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

    protected static Iterable<IndigoRecord> iterateIndigoObject(IndigoObject indigoObject, ErrorHandler errorHandler) {
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
        return iterateSdf(sdfFile, error -> {});
    }

    public static Iterable<IndigoRecord> iterateSdf(String sdfFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSDFile(sdfFile), errorHandler);
    }

    public static Iterable<IndigoRecord> iterateSmiles(String smilesFile) {
        return iterateSmiles(smilesFile, error -> {});
    }

    public static Iterable<IndigoRecord> iterateSmiles(String smilesFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSmilesFile(smilesFile), errorHandler);
    }

    public static Iterable<IndigoRecord> iterateCml(String cmlFile) {
        return iterateCml(cmlFile, error -> {});
    }

    public static Iterable<IndigoRecord> iterateCml(String cmlFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateCMLFile(cmlFile), errorHandler);
    }

}
