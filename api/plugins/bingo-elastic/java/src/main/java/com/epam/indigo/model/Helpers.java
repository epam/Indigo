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
}
