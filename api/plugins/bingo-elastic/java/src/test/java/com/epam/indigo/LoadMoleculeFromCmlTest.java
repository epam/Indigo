package com.epam.indigo;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class LoadMoleculeFromCmlTest {

    @Test
    @DisplayName("Testing creation of IndigoRecord from cml file")
    public void testLoad() throws Exception {
        List<IndigoRecord> indigoRecordList = Helpers.loadFromCmlFile("src/test/resources/tetrahedral-all.cml");
        assertEquals(163, indigoRecordList.size());
    }

}
