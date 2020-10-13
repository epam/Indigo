package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;

import java.util.ArrayList;
import java.util.List;

public class Helpers {

    public static IndigoRecord fromSmiles(String smiles) {
        Indigo indigo = new Indigo();
//        todo
        return null;
    }

    protected static IndigoRecord loadFromIndigoObject(IndigoObject indigoObject) throws Exception {
        IndigoRecord.IndigoRecordBuilder builder = new IndigoRecord.IndigoRecordBuilder().withIndigoObject(
                indigoObject
        );
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        return builder.build();
    }

    public static IndigoRecord loadFromFile(String molfile) throws Exception {
        Indigo indigo = new Indigo();
        IndigoObject object = indigo.loadMoleculeFromFile(molfile);
        return loadFromIndigoObject(object);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile) {
        return loadFromSdf(sdfFile, true);
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

    public static IndigoRecord loadFromSmiles(String smiles) throws Exception {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule(smiles);
        return loadFromIndigoObject(indigoObject);
    }

    public static List<IndigoRecord> loadFromSmilesFile(String smilesFile) {
        Indigo indigo = new Indigo();
        List<IndigoRecord> recordList = new ArrayList<>();
        for (IndigoObject item : indigo.iterateSmilesFile(smilesFile)) {
            try {
                recordList.add(loadFromIndigoObject(item));
            } catch (Exception e) {
                // todo: add error catching here
            }
        }
        return recordList;
    }
}
