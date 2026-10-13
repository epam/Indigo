package com.epam.indigo;


import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
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
    @DisplayName("checkSalt tells a structure with a counter ion from one without")
    void testCheckSalt() {
        Indigo indigo = new Indigo();
        assertTrue(indigo.loadMolecule("CCO.[Na+]").checkSalt());
        assertFalse(indigo.loadMolecule("CCO").checkSalt());
    }

    @Test
    @DisplayName("stripSalt strips a copy by default and the molecule itself when asked")
    void testStripSalt() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("CCO.[Na+]");

        assertEquals("CCO", m.stripSalt().smiles());
        assertEquals("CCO.[Na+]", m.smiles(), "a copy was stripped");

        assertSame(m, m.stripSalt(true));
        assertEquals("CCO", m.smiles());
    }

    @Test
    @DisplayName("an attachment group and a haptic bond are built and read back (#3842)")
    void testHapticBond() {
        Indigo indigo = new Indigo();
        IndigoObject m = indigo.loadMolecule("C1=CC=CC1.[Fe]");
        IndigoObject metal = m.getAtom(5);

        IndigoObject group = m.addAttachmentGroup(new int[] {0, 1, 2, 3, 4});
        IndigoObject bond = m.addHapticBond(group, metal);

        assertEquals(1, m.countAttachmentGroups());
        assertEquals(1, m.countHapticBonds());
        assertEquals(5, group.countAtoms());
        assertEquals("haptic", bond.hapticBondType());
        assertTrue(bond.hapticBondBegin().isAttachmentGroup());
        assertEquals("Fe", bond.hapticBondEnd().symbol());

        // Neither construct is a part of the graph.
        assertEquals(5, m.countBonds());

        // The group goes, and the bond that addressed it goes with it.
        group.remove();
        assertEquals(0, m.countAttachmentGroups());
        assertEquals(0, m.countHapticBonds());
    }
}
