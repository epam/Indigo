package com.epam.indigolucene.common;

import com.epam.indigolucene.common.exceptions.CommitException;
import com.epam.indigolucene.common.exceptions.DocumentAdditionException;
import com.epam.indigolucene.common.query.SolrConnection;
import com.epam.indigolucene.common.types.DocumentRepresentation;
/**
 * Represents upload stream. Maintains such operations as document adding, connection closing.
 *
 * @author Artem Malykh
 * created on 2016-03-29
 */
public class SolrUploadStream<S> implements AutoCloseable {
    private SolrConnection conn;
    private static final int BATCH_SIZE = 1000;

    private int docsInCache;

    public SolrUploadStream(SolrConnection conn) {
        this.conn     = conn;
        docsInCache   = 0;
    }

    @Override
    public void close() throws Exception {
        conn.commit();
        conn.close();
    }

    public <D extends DocumentRepresentation<S>> void  addDocument(D document) throws DocumentAdditionException, CommitException {
        conn.addDocument(document.fieldsMap());
        docsInCache++;
        if (docsInCache >= BATCH_SIZE) {
            conn.commit();
            docsInCache = 0;
        }
    }
}
