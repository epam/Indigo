package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.IndigoRecordReaction;

public class IndexName {

    public enum Index {
        BINGO_MOLECULE,
        BINGO_REACTION;
    }

    public Index index;

    public IndexName(Index index) {
        this.index = index;
    }

    public static IndexName getIndexName(IndigoRecord indigoRecord) {
        if (indigoRecord instanceof IndigoRecordMolecule) {
            return new IndexName(Index.BINGO_MOLECULE);
        }
        if (indigoRecord instanceof IndigoRecordReaction) {
            return new IndexName(Index.BINGO_REACTION);
        }
        throw new BingoElasticException("Unknown IndigoRecord type " + indigoRecord.getClass());
    }

    @Override
    public String toString() {
        return this.index.toString().toLowerCase();
    }

}
