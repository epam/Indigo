package com.epam.indigo.elastic;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecordMolecule;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.Iterator;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

public class IterateOnFileTest {

    // Helpers.iterateSdf/iterateSmiles/iterateCml each create their own internal Indigo session.
    // CMF is a self-contained binary format, so cross-session deserialization via getIndigoObject()
    // works correctly as long as both sessions use default options.
    static Indigo indigo;

    @BeforeAll
    static void initIndigo() {
        indigo = new Indigo();
    }

    @Test
    @DisplayName("SDF iterator: round-trip canonical SMILES equivalence for all 371 molecules")
    void testIterateSdf() {
        String sdfFile = "src/test/resources/rand_queries_small.sdf";
        Iterable<IndigoRecordMolecule> indigoRecords = Helpers.iterateSdf(sdfFile);
        Iterator<IndigoRecordMolecule> recordIterator = indigoRecords.iterator();
        int count = 0;
        for (IndigoObject indigoObject : indigo.iterateSDFile(sdfFile)) {
            IndigoRecordMolecule indigoRecord = recordIterator.next();
            indigoObject.aromatize();
            String expected = indigoObject.canonicalSmiles();
            String actual = indigoRecord.getIndigoObject(indigo).canonicalSmiles();
            assertEquals(expected, actual, "Canonical SMILES mismatch at molecule index " + count);
            count++;
        }
        assertFalse(recordIterator.hasNext(), "Helpers iterator has more records than native iterator");
        assertEquals(371, count, "Expected 371 molecules in SDF file");
    }

    @Test
    @DisplayName("SMILES iterator: round-trip canonical SMILES equivalence for 50 molecules")
    void testIterateSmiles() {
        String smilesFile = "src/test/resources/pubchem_slice_50.smi";
        Iterable<IndigoRecordMolecule> indigoRecords = Helpers.iterateSmiles(smilesFile);
        Iterator<IndigoRecordMolecule> recordIterator = indigoRecords.iterator();
        int count = 0;
        for (IndigoObject indigoObject : indigo.iterateSmilesFile(smilesFile)) {
            IndigoRecordMolecule indigoRecord = recordIterator.next();
            indigoObject.aromatize();
            String expected = indigoObject.canonicalSmiles();
            String actual = indigoRecord.getIndigoObject(indigo).canonicalSmiles();
            assertEquals(expected, actual, "Canonical SMILES mismatch at molecule index " + count);
            count++;
        }
        assertFalse(recordIterator.hasNext(), "Helpers iterator has more records than native iterator");
        assertEquals(50, count, "Expected 50 molecules in SMILES file");
    }

    @Test
    @DisplayName("CML iterator: round-trip canonical SMILES equivalence for 163 molecules")
    void testIterateCml() {
        String cmlFile = "src/test/resources/tetrahedral-all.cml";
        Iterable<IndigoRecordMolecule> indigoRecords = Helpers.iterateCml(cmlFile);
        Iterator<IndigoRecordMolecule> recordIterator = indigoRecords.iterator();
        int count = 0;
        for (IndigoObject indigoObject : indigo.iterateCMLFile(cmlFile)) {
            IndigoRecordMolecule indigoRecord = recordIterator.next();
            indigoObject.aromatize();
            String expected = indigoObject.canonicalSmiles();
            String actual = indigoRecord.getIndigoObject(indigo).canonicalSmiles();
            assertEquals(expected, actual, "Canonical SMILES mismatch at molecule index " + count);
            count++;
        }
        assertFalse(recordIterator.hasNext(), "Helpers iterator has more records than native iterator");
        assertEquals(163, count, "Expected 163 molecules in CML file");
    }

    @Test
    @DisplayName("SDF iterator: IndigoRecordMolecule fields are populated after conversion")
    void testIterateSdfRecordIntegrity() {
        String sdfFile = "src/test/resources/rand_queries_small.sdf";
        Iterator<IndigoRecordMolecule> it = Helpers.iterateSdf(sdfFile).iterator();
        assertTrue(it.hasNext(), "Iterator should have at least one record");
        IndigoRecordMolecule indigoRecord = it.next();
        assertNotNull(indigoRecord.getCmf(), "CMF bytes should not be null");
        assertTrue(indigoRecord.getCmf().length > 0, "CMF bytes should not be empty");
        assertNotNull(indigoRecord.getSimFingerprint(), "Sim fingerprint should not be null");
        assertNotNull(indigoRecord.getSubFingerprint(), "Sub fingerprint should not be null");
        IndigoObject restored = indigoRecord.getIndigoObject(indigo);
        assertNotNull(restored, "Deserialized IndigoObject should not be null");
        assertFalse(restored.canonicalSmiles().isEmpty(), "Canonical SMILES should not be empty");
    }

}
