package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;

import java.util.ArrayList;
import java.util.List;


class Accumulate {

    protected List<IndigoRecord> acc;
    protected Boolean skipErrors;

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
}
