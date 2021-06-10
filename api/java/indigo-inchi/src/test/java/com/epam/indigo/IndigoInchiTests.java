package com.epam.indigo;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.assertEquals;


public class IndigoInchiTests {

    @Test
    @DisplayName("Loading molecule from string and getting inchi")
    void loadMoleculeFromSMILES() {
        Indigo indigo = new Indigo(Paths.get(System.getProperty("user.dir"), "..", "..", "..", "dist", "lib").normalize().toAbsolutePath().toString());
        IndigoInchi indigoInchi = new IndigoInchi(indigo);
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");
        String inchi = indigoInchi.getInchi(indigoObject);

        assertEquals(
                "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H",
                inchi,
                "C1=CC=CC=C1 have following inchi InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H");
    }
}
