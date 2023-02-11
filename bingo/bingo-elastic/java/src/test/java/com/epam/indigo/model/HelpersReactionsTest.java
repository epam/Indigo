package com.epam.indigo.model;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import org.junit.Test;

import static org.junit.jupiter.api.Assertions.*;


public class HelpersReactionsTest {

    @Test
    public void LoadReactionTest() {
        Indigo indigo = new Indigo();
        String fileReactionPath = "src/test/resources/reactions/rheadb/58029.rxn";
        IndigoObject indigoReaction = indigo.loadReactionFromFile(fileReactionPath);
        IndigoRecordReaction indigoRecordReaction = Helpers.loadReaction(fileReactionPath);
        IndigoObject indigoObjectFromRecord = indigoRecordReaction.getIndigoObject(indigo);
        assertEquals(indigoReaction.countReactants(), indigoObjectFromRecord.countReactants());
        assertEquals(indigoReaction.countProducts(), indigoObjectFromRecord.countProducts());
    }

}
