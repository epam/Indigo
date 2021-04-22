package com.epam.indigo;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.bulk.BulkResponse;

import java.io.IOException;
import java.util.stream.Stream;

public interface GenericRepository<T extends IndigoRecord> {

    Stream<T> stream();

    void indexRecords(Iterable<T> records, int batchSize) throws IOException;

    void indexRecords(Iterable<T> records, int batchSize, ActionListener<BulkResponse> actionListener) throws IOException;

    boolean deleteAllRecords() throws IOException;

}
