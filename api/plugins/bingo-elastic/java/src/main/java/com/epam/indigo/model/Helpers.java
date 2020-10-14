package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord.IndigoRecordBuilder;

import java.util.ArrayList;
import java.util.List;

public class Helpers {

    protected static IndigoRecord loadFromIndigoObject(IndigoObject indigoObject) throws Exception {
        IndigoRecordBuilder builder = new IndigoRecordBuilder().withIndigoObject(
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

    public static List<IndigoRecord> loadFromSdf(String sdfFile) throws Exception {
        return loadFromSdf(sdfFile, true);
    }

    public static List<IndigoRecord> loadFromSdf(String sdfFile, Boolean skipErrors) throws Exception {
        Indigo indigo = new Indigo();
        List<IndigoRecord> recordList = new ArrayList<>();
        for (IndigoObject comp : indigo.iterateSDFile(sdfFile)) {
            try {
                recordList.add(loadFromIndigoObject(comp));
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
