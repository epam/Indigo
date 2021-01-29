package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;

import java.util.*;


/**
 * Helpers class, that have ability to create {@link com.epam.indigo.model.IndigoRecord} from popular formats like SDF, MOL, Smiles, etc
 */
public class Helpers {

    ////////////////////////////////////////////////////////////////
    //
    // MOLECULE HELPERS
    //
    ////////////////////////////////////////////////////////////////


    protected static IndigoRecordMolecule load(IndigoObject comp) {
        return load(comp, error -> {});
    }

    protected static IndigoRecordMolecule load(IndigoObject comp, ErrorHandler errorHandler) {
        return FromIndigoObject.buildMolecule(comp, errorHandler);
    }

    public static IndigoRecordMolecule loadMolecule(String molFile) {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMoleculeFromFile(molFile);
        return FromIndigoObject.buildMolecule(indigoObject);
    }

    public static IndigoRecordMolecule loadFromSmiles(String smiles) {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule(smiles);
        return FromIndigoObject.buildMolecule(indigoObject);
    }

    public static Iterable<IndigoRecordMolecule> iterateSdf(String sdfFile) {
        return iterateSdf(sdfFile, error -> {});
    }

    public static Iterable<IndigoRecordMolecule> iterateSdf(String sdfFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSDFile(sdfFile), errorHandler);
    }

    public static Iterable<IndigoRecordMolecule> iterateSmiles(String smilesFile) {
        return iterateSmiles(smilesFile, error -> {});
    }

    public static Iterable<IndigoRecordMolecule> iterateSmiles(String smilesFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateSmilesFile(smilesFile), errorHandler);
    }

    public static Iterable<IndigoRecordMolecule> iterateCml(String cmlFile) {
        return iterateCml(cmlFile, error -> {});
    }

    public static Iterable<IndigoRecordMolecule> iterateCml(String cmlFile, ErrorHandler errorHandler) {
        Indigo indigo = new Indigo();
        return iterateIndigoObject(indigo.iterateCMLFile(cmlFile), errorHandler);
    }

    ////////////////////////////////////////////////////////////////
    //
    // REACTION HELPERS
    //
    ////////////////////////////////////////////////////////////////

    public static IndigoRecordReaction loadReaction(String file) {
        Indigo indigo = new Indigo();
        return FromIndigoObject.buildReaction(indigo.loadReactionFromFile(file));
    }

    // TODO: Generalize next two methods
    // TODO: Move from helpers
    public static IndigoRecordMolecule moleculeFromElastic(String id, Map<String, Object> source, float score) throws BingoElasticException {

        IndigoRecordMolecule.IndigoRecordBuilder indigoRecordBuilder = new IndigoRecordMolecule.IndigoRecordBuilder();
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

    public static IndigoRecordReaction reactionFromElastic(String id, Map<String, Object> source, float score) throws BingoElasticException {

        IndigoRecordReaction.IndigoRecordBuilder indigoRecordBuilder = new IndigoRecordReaction.IndigoRecordBuilder();
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

    protected static Iterable<IndigoRecordMolecule> iterateIndigoObject(IndigoObject indigoObject, ErrorHandler errorHandler) {
        return () -> new Iterator<IndigoRecordMolecule>() {
            @Override
            public boolean hasNext() {
                return indigoObject.hasNext();
            }

            @Override
            public IndigoRecordMolecule next() {
                return load(indigoObject.next());
            }
        };
    }

}
