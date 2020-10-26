package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord.IndigoRecordBuilder;

import java.util.ArrayList;
import java.util.Base64;
import java.util.List;
import java.util.Map;


/**
 * Helpers class, that have ability to create {@link com.epam.indigo.model.IndigoRecord} from popular formats like SDF, MOL, Smiles, etc
 */
public class Helpers {

    protected static void loadOrThrow(List<IndigoRecord> acc, IndigoObject comp, Boolean skipErrors) {
        try {
            comp.aromatize();
            acc.add(FromIndigoObject.build(comp));
        } catch (Exception e) {
            if (!skipErrors) {
                throw e;
            }
        }
    }

    public static IndigoRecord loadFromFile(String molFile) {
        Indigo indigo = new Indigo();
        return FromIndigoObject.build(indigo.loadMoleculeFromFile(molFile));
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile) throws Exception {
        return loadFromSdf(sdfFile, true);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile, Boolean skipErrors) throws Exception {
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

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile, Boolean skipErrors) throws Exception {
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

    public static List<IndigoRecord> loadFromCmlFile(String cmlFile, Boolean skipErrors) throws Exception {
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
            if (entry.getKey().equals("fingerprint")) {
                indigoRecordBuilder.withFingerprint((List<Integer>) ((List<Object>) entry.getValue()).get(0));
            } else {
                indigoRecordBuilder.withCustomObject(entry.getKey(), entry.getValue());
            }
        }
        indigoRecordBuilder.withScore(score);
        indigoRecordBuilder.withId(id);
        byte[] cmf = Base64.getDecoder().decode((String) source.get("cmf"));
        indigoRecordBuilder.withCmf(cmf);
        indigoRecordBuilder.withName((String) source.get("name"));
        return indigoRecordBuilder.build();
    }
}
