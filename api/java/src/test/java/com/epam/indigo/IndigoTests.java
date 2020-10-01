package com.epam.indigo;


import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IndigoTests {

    @Test
    @DisplayName("Loading molecule from string and comparing canonical smiles")
    void loadMoleculeFromSMILES() {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

        assertEquals(
                "C1C=CC=CC=1",
                indigoObject.canonicalSmiles(),
                "C1=CC=CC=C1 is the same as C1C=CC=CC=1");
    }

    @Test
    @DisplayName("Loading molecule and getting one bits list")
    void getOneBitsList() {
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");

        assertEquals(
                "1698 1719 1749 1806 1909 1914 1971 2056",
                indigoObject.fingerprint().oneBitsList(),
                "same one bits as in string 1698 1719 1749 1806 1909 1914 1971 205");
    }
}
