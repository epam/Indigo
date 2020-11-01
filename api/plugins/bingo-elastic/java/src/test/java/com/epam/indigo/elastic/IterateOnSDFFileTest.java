package com.epam.indigo.elastic;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.Iterator;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IterateOnSDFFileTest {


    @Test
    @DisplayName("Testing iterator over sdf file")
    void testLoadFromMol() throws Exception {
        Iterable<IndigoRecord> indigoRecords = Helpers.iterateSdf("src/test/resources/rand_queries_small.sdf");
        int count = 0;
        for (IndigoRecord indigoRecord : indigoRecords) {
            count++;
        }
        assertEquals(371, count);
    }
}
