package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class LoadMoleculeFromSmilesTest {

    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles")
    void testLoad() {
        try {
            String smiles = "O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C";
            IndigoRecord indigoRecord = Helpers.loadFromSmiles(smiles);
            Indigo session = new Indigo();
            IndigoObject indigoObject = session.loadMolecule(smiles);
            IndigoObject targetIndigoObject = indigoRecord.getIndigoObject(session);
            // todo: uncomment
            //assertEquals(indigoObject.fingerprint(), targetIndigoObject.fingerprint());
        } catch (Exception e) {
            Assertions.fail();
        }
    }

    @Test
    @DisplayName("Testing creation of IndigoRecord from smiles file")
    void testLoadFromFile() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSmilesFile(
                    "src/test/resources/pubchem_slice_50.smi"
            );
            assertEquals(50, indigoRecordList.size());
        } catch (Exception e) {
            Assertions.fail();
        }
    }

}
