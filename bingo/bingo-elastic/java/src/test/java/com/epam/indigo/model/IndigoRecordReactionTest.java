package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import org.junit.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class IndigoRecordReactionTest {

    @Test
    public void builderTest() {
        IndigoRecordReaction.IndigoRecordBuilder builder = new IndigoRecordReaction.IndigoRecordBuilder();
        Indigo indigo = new Indigo();
        IndigoObject indigoReaction = indigo.loadReactionFromFile("src/test/resources/reactions/q_43.rxn");
        builder.withIndigoObject(indigoReaction);
        IndigoRecordReaction recordReaction = builder.build();
        assertEquals(indigoReaction.countReactants(), recordReaction.getIndigoObject(indigo).countReactants());
    }

}
