package com.epam.indigo.elastic;

import static org.junit.jupiter.api.Assertions.*;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.IndigoRecordReaction;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

public class IndexNameTest {

    @Test
    public void indexName() {
        Map<String, IndexName.Index> testPairs = new HashMap<>();
        testPairs.put("bingo_molecule", IndexName.Index.BINGO_MOLECULE);
        testPairs.put("bingo_reaction", IndexName.Index.BINGO_REACTION);
        for (Map.Entry<String, IndexName.Index> testPair : testPairs.entrySet()) {
            IndexName indexName = new IndexName(
                    testPair.getValue()
            );
            assertEquals(indexName.toString(), testPair.getKey());
        }
    }

    @Test
    public void getIndexNameByMolecule() {

        IndigoRecordMolecule.IndigoRecordBuilder builder = new IndigoRecordMolecule.IndigoRecordBuilder();
        Indigo indigo = new Indigo();
        IndigoObject indigoObject = indigo.loadMolecule("O(C(C[N+](C)(C)C)CC([O-])=O)C(=O)C");
        builder.withIndigoObject(indigoObject);
        IndigoRecordMolecule recordMolecule = builder.build();

        IndexName indexName = IndexName.getIndexName(recordMolecule);
        assertEquals(indexName.index, IndexName.Index.BINGO_MOLECULE);
    }

    @Test
    public void getIndexNameByReaction() {
        IndigoRecordReaction.IndigoRecordBuilder builder = new IndigoRecordReaction.IndigoRecordBuilder();
        Indigo indigo = new Indigo();
        IndigoObject indigoReaction = indigo.loadReactionFromFile("src/test/resources/reactions/q_43.rxn");
        builder.withIndigoObject(indigoReaction);
        IndigoRecordReaction recordReaction = builder.build();

        IndexName indexName = IndexName.getIndexName(recordReaction);
        assertEquals(indexName.index, IndexName.Index.BINGO_REACTION);
    }


}
