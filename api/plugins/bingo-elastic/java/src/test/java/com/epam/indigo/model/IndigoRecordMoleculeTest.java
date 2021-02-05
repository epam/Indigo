package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import org.junit.Test;

import static org.junit.jupiter.api.Assertions.*;


public class IndigoRecordMoleculeTest {

    @Test
    public void builderTest() {
        IndigoRecordMolecule.IndigoRecordBuilder builder = new IndigoRecordMolecule.IndigoRecordBuilder();
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule("O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C");
        builder.withIndigoObject(indigoObject);
        IndigoRecordMolecule recordMolecule = builder.build();
        assertEquals(recordMolecule.getIndigoObject(indigo).canonicalSmiles(), indigoObject.canonicalSmiles());
    }

}
