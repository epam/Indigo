package com.epam.indigo;


import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

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
    @DisplayName("checkSalt detects mono-, di-, tri- and tetravalent monoatomic cations")
    void testCheckSaltMonoatomicCations() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("[Na+].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Rb+].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Ca+2].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Zn+2].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Al+3].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Cr+3].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Ru+4].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Sn+4].C").checkSalt());
    }

    @Test
    @DisplayName("checkSalt detects mono- and divalent monoatomic anions")
    void testCheckSaltMonoatomicAnions() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("[Cl-].C").checkSalt());
        assertTrue(indigo.loadMolecule("[F-].C").checkSalt());
        assertTrue(indigo.loadMolecule("[S-2].C").checkSalt());
        assertTrue(indigo.loadMolecule("[Se-2].C").checkSalt());
    }

    @Test
    @DisplayName("checkSalt detects molecular inorganic salts")
    void testCheckSaltMolecularSalts() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("S=[Fe].C").checkSalt());
        assertTrue(indigo.loadMolecule("Cl[Ag]").checkSalt());
        assertTrue(indigo.loadMolecule("S=[Sn]=S.C").checkSalt());
        assertTrue(indigo.loadMolecule("O=[Mn]=O.C").checkSalt());
        assertTrue(indigo.loadMolecule("Cl[Fe](Cl)Cl.C").checkSalt());
        assertTrue(indigo.loadMolecule("OCl(=O)=O.C").checkSalt());
        assertTrue(indigo.loadMolecule("OS(=O)(=O)O.C").checkSalt());
        assertTrue(indigo.loadMolecule("OP(=O)(O)O.C").checkSalt());
    }

    @Test
    @DisplayName("checkSalt detects complex inorganic ions")
    void testCheckSaltComplexIons() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("[OH-].C").checkSalt());
        assertTrue(indigo.loadMolecule("[O-]Cl.C").checkSalt());
        assertTrue(indigo.loadMolecule("[O-]I=O.C").checkSalt());
        assertTrue(indigo.loadMolecule("[O-]N(=O).C").checkSalt());
        assertTrue(indigo.loadMolecule("[N+](=O)([O-])[O-].C").checkSalt());
        assertTrue(indigo.loadMolecule("O[Se](=O)[O-].C").checkSalt());
        assertTrue(indigo.loadMolecule("OP(=O)(O)[O-].C").checkSalt());
        assertTrue(indigo.loadMolecule("OS(=O)(=O)[O-].C").checkSalt());
    }

    @Test
    @DisplayName("checkSalt detects structures containing several ions")
    void testCheckSaltMultipleIons() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("[Na+].[Cl-].C").checkSalt());
        assertTrue(indigo.loadMolecule("[O-]S(=O)(=O)[O-].[K+].[K+].C").checkSalt());
    }

    @Test
    @DisplayName("checkSalt returns false for structures without disconnected inorganic components")
    void testCheckSaltNoIons() {
        Indigo indigo = new Indigo();
        assertFalse(indigo.loadMolecule("c1ccccc1").checkSalt());
        assertFalse(indigo.loadMolecule("C1=CC=C2C=CC=CC2=C1").checkSalt());
    }

    @Test
    @DisplayName("checkSalt returns false when the metal or acid group is bonded to an organic part")
    void testCheckSaltBonded() {
        Indigo indigo = new Indigo();
        assertFalse(indigo.loadMolecule("CC[Pb](CC)(CC)CC").checkSalt());
        assertFalse(indigo.loadMolecule("C[Al](C)C").checkSalt());
        assertFalse(indigo.loadMolecule("C1=CC=C(C=C1)[N+](=O)[O-]").checkSalt());
        assertFalse(indigo.loadMolecule("C(C(=O)O)S(=O)(=O)O").checkSalt());
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
