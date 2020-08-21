package com.epam.indigo;

import java.nio.file.Paths;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IndigoTests {

    @Test
    @DisplayName("Loading molecule from string and comparing canonical smiles")
    void loadMoleculeFromSMILES() {
        Indigo indigo = new Indigo(Paths.get(System.getProperty("user.dir"), "..", "libs", "shared").normalize().toAbsolutePath().toString());
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

        assertEquals(
                "C1C=CC=CC=1",
                indigoObject.canonicalSmiles(),
                "C1=CC=CC=C1 is the same as C1C=CC=CC=1");
    }
}
