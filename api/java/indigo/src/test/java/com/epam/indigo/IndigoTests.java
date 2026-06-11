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

    @Test
    @DisplayName("Copies R-group from one molecule to another")
    void testCopyRGroups() {
        Indigo indigo = new Indigo();
        IndigoObject molWithRg = indigo.loadMolecule(
            "C%91C.[*:1]%91 |$;;_R1$,RG:_R1={F%91.Cl%92.Br%93.[*:1]%91.[*:1]%92.[*:1]%93 |$;;;_AP1;_AP1;_AP1$|}|"
        );
        assertEquals(molWithRg.countRGroups(), 1);
        IndigoObject molWithNoRg = indigo.loadMolecule(
            "C%91C.[*:1]%91 |$;;_R1$|"
        );
        assertEquals(molWithNoRg.countRGroups(), 0);
        molWithRg.copyRGroups(molWithNoRg);
        assertEquals(molWithNoRg.countRGroups(), 1);
    }

    @Test
    @DisplayName("stripSalt keeps a molecule without disconnected inorganic components intact")
    void testStripSaltNoSalts() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1");
        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1", m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt removes a single inorganic anion")
    void testStripSaltSingleSalt() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]");
        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1", m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt removes many inorganic components (water and anions)")
    void testStripSaltManySalts() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.O.O.O.O.O.O.O.O.O.O.[Cl-].[Cl-]");
        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1", m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt keeps multiple organic components and removes the anion")
    void testStripSaltTwoOrganics() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule(
            "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[O-]S(=O)(=O)[O-]");
        assertEquals(
            "CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1",
            m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt keeps organic acids and a lone metal atom")
    void testStripSaltComplexSalt() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule(
            "C(C(C(C(C(C(=O)O)O)O)O)O)O.C(C(C(C(C(C(=O)O)O)O)O)O)O.[Fe]");
        assertEquals(
            "C(O)C(O)C(O)C(O)C(O)C(O)=O.C(O)C(O)C(O)C(O)C(O)C(O)=O.[Fe]",
            m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt yields an empty molecule when only inorganic components are present")
    void testStripSaltOnlySalts() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("[NH4+].[O-]P(=O)([O-])[O-].[Fe+2]");
        assertEquals("", m.stripSalt().smiles());
    }

    @Test
    @DisplayName("stripSalt inplace=false leaves the original untouched; inplace=true mutates it")
    void testStripSaltOptions() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]");
        IndigoObject mStrip = m.stripSalt();

        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1", mStrip.smiles());
        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1.[Cl-]", m.smiles());

        m.stripSalt(true);
        assertEquals("CCCCCCCCCCCCCCCC[N+]1=CC=CC=C1", m.smiles());
    }
}
