package com.epam.indigo;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IndigoRendererTests {

    @Test
    @DisplayName("Testing indigo renderer")
    @Disabled("Need to figure out what mode is")
    void renderIndigoObjects() {
        Indigo indigo = new Indigo(System.getProperty("user.dir") + "/../../libs/shared");
        IndigoRenderer indigoRenderer = new IndigoRenderer(indigo);
        IndigoObject indigoObject = indigo.loadMolecule("C1=CC=CC=C1");
        byte[] bytes = indigoRenderer.renderToBuffer(indigoObject);


        assertEquals(
                null,
                bytes,
                "rendered bytes should be the same"
        );
    }
}
