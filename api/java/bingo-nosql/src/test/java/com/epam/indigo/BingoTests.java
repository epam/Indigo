package com.epam.indigo;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;
import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class BingoTests {
    @Test
    @DisplayName("Testing loading of the bingo dll")
    void checkBingoVersion(@TempDir Path tempDir) {
        Indigo indigo = new Indigo(Paths.get(System.getProperty("user.dir"), "..", "..", "..", "dist", "lib").normalize().toAbsolutePath().toString());
        Bingo bingo = Bingo.createDatabaseFile(indigo, tempDir.toString(), "molecule", "");
        assertEquals(
                "v0.72",
                bingo.version(),
                "Checking version of the Bingo"
        );
        bingo.close();
    }

    @Test
    @DisplayName("Testing enumerateId from Java bindings")
    void checkEnumerateId(@TempDir Path tempDir) {
        Indigo indigo = new Indigo();
        Bingo bingo = Bingo.createDatabaseFile(indigo, tempDir.toString(), "molecule");
        bingo.insert(indigo.loadMolecule("C"));
        bingo.insert(indigo.loadMolecule("CC"));
        BingoObject enumerator = bingo.enumerateId();
        int i = 0;
        do {
            assertEquals(
                i,
                enumerator.getCurrentId(),
                "Checking currentId"
            );
            i++;
        } while (enumerator.next());
    }
}
