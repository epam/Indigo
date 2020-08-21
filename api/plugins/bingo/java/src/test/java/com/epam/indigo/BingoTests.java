package com.epam.indigo;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class BingoTests {

    @Test
    @DisplayName("Testing loading of the bingo dll")
    @Disabled
    void checkBingoVersion() {
        Indigo indigo = new Indigo(Paths.get(System.getProperty("user.dir"), "..", "..", "..", "libs", "shared").normalize().toAbsolutePath().toString());
        Bingo bingo = new Bingo(indigo, "", "");
        assertEquals(
                "",
                bingo.version(),
                "Checking version of the Bingo"
        );
    }

}
