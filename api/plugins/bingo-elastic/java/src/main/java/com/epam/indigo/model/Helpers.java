package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord.IndigoRecordBuilder;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;


class Accumulate {

    protected final List<IndigoRecord> acc;
    protected final Boolean skipErrors;

    public Accumulate(Boolean skipErrors) {
        acc = new ArrayList<>();
        this.skipErrors = skipErrors;
    }

    public void add(IndigoRecord record) throws Exception {
        try {
            acc.add(record);
        } catch (Exception e) {
            if (!skipErrors) {
                throw e;
            }
        }
    }

    public List<IndigoRecord> getAcc() {
        return acc;
    }

}

/**
 * Helpers class, that have ability to create @link {com.epam.indigo.model.IndigoRecord} from popular formats like SDF, MOL, Smiles, etc
 */
public class Helpers {


    public static IndigoRecord loadFromFile(String molfile) {
        Indigo indigo = new Indigo();
        return (new FromIndigoObject(indigo.loadMoleculeFromFile(molfile))).get();
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile) throws Exception {
        return loadFromSdf(sdfFile, true);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile, Boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        Accumulate acc = new Accumulate(skipErrors);
        for (IndigoObject comp : indigo.iterateSDFile(sdfFile)) {
            acc.add((new FromIndigoObject(comp)).get());
        }
        return acc.getAcc();
    }

    public static IndigoRecord loadFromSmiles(String smiles) throws Exception {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule(smiles);
        return (new FromIndigoObject(indigoObject)).get();
    }

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile) throws Exception {
        return loadFromSmilesFile(smilesFile, true);
    }

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile, Boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        Accumulate acc = new Accumulate(skipErrors);
        for (IndigoObject comp : indigo.iterateSmilesFile(smilesFile)) {
            acc.add((new FromIndigoObject(comp)).get());
        }
        return acc.getAcc();
    }

    public static List<IndigoRecord> loadFromCmlFile(String cmlFile) throws Exception {
        return loadFromCmlFile(cmlFile, true);
    }

    public static List<IndigoRecord> loadFromCmlFile(String cmlFile, Boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        Accumulate acc = new Accumulate(skipErrors);
        for (IndigoObject comp : indigo.iterateCMLFile(cmlFile)) {
            acc.add((new FromIndigoObject(comp)).get());
        }
        return acc.getAcc();
    }

    public static IndigoRecord fromSource(String id, Map<String, Object> source, float score) {
        Indigo indigo = new Indigo();
        IndigoRecordBuilder indigoRecordBuilder = new IndigoRecordBuilder();
        for (Map.Entry<String, Object> entry : source.entrySet()) {
            if (entry.getKey().equals("fingerprint")) {
                indigoRecordBuilder.withFingerprint((List<Integer>) ((List<Object>)entry.getValue()).get(0));
            } else {
                indigoRecordBuilder.withCustomObject(entry.getKey(), entry.getValue());
            }
        }
        indigoRecordBuilder.withScore(score);
        indigoRecordBuilder.withId(id);
        Object cmf = source.get("cmf");
        indigoRecordBuilder.withCmf(cmf);
        return indigoRecordBuilder.build();
    }
}
