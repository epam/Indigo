package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class LoadMoleculeFromSdfTest {

    @Test
    @DisplayName("Testing creation of IndigoRecord from sdf file")
    void testLoad() throws Exception {
        List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
        assertEquals(371, indigoRecordList.size());
    }

}
