package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;

/**
 * Class for storing/retrieving molecules from Elasticsearch
 */
public class IndigoRecordMolecule extends IndigoRecord {

    public static class IndigoRecordBuilder extends IndigoRecord.IndigoRecordBuilder<IndigoRecordMolecule> {

        public IndigoRecordMolecule build() throws BingoElasticException {
            IndigoRecordMolecule record = new IndigoRecordMolecule();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

    }

}
