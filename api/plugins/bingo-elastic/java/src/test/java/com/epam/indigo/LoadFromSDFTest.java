package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class LoadFromSDFTest {

    @Test
    @DisplayName("Testing creation of creation of IndigoRecord")
    void testLoad() throws Exception {
        IndigoRecord indigoRecord = Helpers.loadFromFile("src/test/resources/composition1.mol");
        assertEquals(null, indigoRecord.getFingerprint());
    }
}
