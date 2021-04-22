package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;

/**
 * Class for storing/retrieving reactions from Elasticsearch
 */
public class IndigoRecordReaction extends IndigoRecord {

    public static class IndigoRecordBuilder extends IndigoRecord.IndigoRecordBuilder<IndigoRecordReaction> {

        public IndigoRecordReaction build() throws BingoElasticException {
            IndigoRecordReaction record = new IndigoRecordReaction();
            operations.forEach(operation -> operation.accept(record));
            validate(record);
            return record;
        }

    }
}
