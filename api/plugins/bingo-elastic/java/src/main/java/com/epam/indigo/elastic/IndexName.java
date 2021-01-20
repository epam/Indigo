package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.IndigoRecordReaction;

public class IndexName {

    public static final String BINGO_MOLECULE = "bingo_molecules";
    public static final String BINGO_REACTION = "bingo_reactions";

    private final String index;

    public IndexName(String index) {
        this.index = index;
    }

    public static IndexName getIndexName(IndigoRecord indigoRecord) {
        if (indigoRecord instanceof IndigoRecordMolecule) {
            return new IndexName(BINGO_MOLECULE);
        }
        if (indigoRecord instanceof IndigoRecordReaction) {
            return new IndexName(BINGO_REACTION);
        }
        throw new BingoElasticException("Unknown IndigoRecord type " + indigoRecord.getClass());
    }

    @Override
    public String toString() {
        return this.index;
    }

}
