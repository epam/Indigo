package com.epam.indigolucene.common.query;

import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.types.fields.Field;
import com.epam.indigolucene.common.types.conditions.Condition;

import org.json.simple.JSONObject;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Consumer;
/**
 *  Main purpose of this class is to provide an easy query manipulation mechanisms, such as field inclusion, field
 *  deletion, set of return results limit, offset, Solr-server connection instantiation and result processing.
 *
 * @author Artem Malykh
 * created on 2016-02-24
 */
//TODO: This is a service class, should not be public
public class BeforeGroup<S> {
    public Query getQuery() {
        return query;
    }

    protected Query query;
    protected CollectionRepresentation<S> collection;

    public BeforeGroup(Set<String> fields, CollectionRepresentation<S> collection) throws IllegalAccessException, InstantiationException {
        query = new Query(fields);
        this.collection = collection;
    }

    public BeforeGroup<S> filter(Condition<S> c) {
        if (query.condition == null) {
            query.condition = c;
        } else {
            query.condition = query.condition.and(c);
        }
        return this;
    }

    public BeforeGroup<S> includeField(Field<S, ?, ?> field) {
        query.includeField.add(field.getName());
        return this;
    }

    public BeforeGroup<S> dropField(Field<S, ?, ?> field) {
        query.dropField.add(field.getName());
        return this;
    }

    public BeforeGroup<S> offset(int offset) {
        query.offset = offset;
        return this;
    }

    public BeforeGroup<S> limit(int limit) {
        query.limit = limit;
        return this;
    }

    public JSONObject toJson() {
        return query.toJson();
    }

    private SolrConnection getSolrConnection() throws InstantiationException, IllegalAccessException {
        SolrConnection res = SolrConnectionFactory.createInstance();
        res.setUrl(collection.getUrl());
        res.setCoreName(collection.getCoreName());
        res.setQuery(query);

        return res;
    }

    public void processWith(Consumer<List<Map<String, Object>>> processor) throws Exception {
        SolrConnection solrConnection = getSolrConnection();
        processor.accept(solrConnection.results());
    }
}
