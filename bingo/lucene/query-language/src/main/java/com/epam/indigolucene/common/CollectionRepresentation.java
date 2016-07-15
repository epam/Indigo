package com.epam.indigolucene.common;

import com.epam.indigolucene.common.exceptions.DocumentAdditionException;
import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.BeforeGroup;
import com.epam.indigolucene.common.query.SolrConnection;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.common.types.DocumentRepresentation;

import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by Artem Malykh on 28.03.16.
 */
public class CollectionRepresentation<S> {


    private String url;
    private String coreName;
    Set<String> fieldNames;

    public CollectionRepresentation(String url, String coreName, Set<String> fieldNames) {
        this.url = url;
        this.fieldNames = fieldNames;
        this.coreName = coreName;
    }

    public BeforeGroup<S> find() throws InstantiationException, IllegalAccessException {
        return new BeforeGroup<>(fieldNames, this);
    }

    public void removeAll() throws RemoveException {
        try {
            createConnection().removeAll();
        } catch (Exception e) {
            throw new RemoveException(e);
        }
    }

    public SolrUploadStream<S> uploadStream() throws InstantiationException, IllegalAccessException {
        return new SolrUploadStream<>(createConnection());
    }

    public <D extends DocumentRepresentation<S>> void addDocument(D doc) throws DocumentAdditionException {
        try {
            try (SolrUploadStream<S> ustream = uploadStream()) {
                ustream.addDocument(doc);
            }
        } catch (Exception e) {
            throw new DocumentAdditionException(doc.fieldsMap(), e);
        }
    }

    private SolrConnection createConnection() throws InstantiationException, IllegalAccessException {
        SolrConnection conn = SolrConnectionFactory.createInstance();
        conn.setUrl(url);
        conn.setCoreName(coreName);
        return conn;
    }

    public String getUrl() {
        return url;
    }

    public String getCoreName() {
        return coreName;
    }
}
