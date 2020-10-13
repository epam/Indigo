package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertNotNull;

public class LoadMoleculeFromFileTest {
    @Test
    @DisplayName("Testing creation of IndigoRecord from mol file")
    void testLoad() throws Exception {
        IndigoRecord indigoRecord = Helpers.loadFromFile("src/test/resources/composition1.mol");
        assertNotNull(indigoRecord.getFingerprint());
    }
}
