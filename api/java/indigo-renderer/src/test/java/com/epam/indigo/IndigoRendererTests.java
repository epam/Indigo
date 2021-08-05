package com.epam.indigo;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IndigoRendererTests {

    @Test
    @DisplayName("Testing indigo renderer")
    void renderIndigoObjects() {
        Indigo indigo = new Indigo(Paths.get(System.getProperty("user.dir"), "..", "..", "..", "dist", "lib").normalize().toAbsolutePath().toString());
        IndigoRenderer indigoRenderer = new IndigoRenderer(indigo);
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");
        indigo.setOption("render-output-format", "svg");
        byte[] bytes = indigoRenderer.renderToBuffer(indigoObject);

        assertEquals(
                60,
                bytes[0],
                "first rendered bytes should be the same"
        );
        assertEquals(
                10,
                bytes[bytes.length - 1],
                "last rendered bytes should be the same"
        );
    }
}
