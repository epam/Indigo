package com.epam.indigolucene.solrconnection.elastic;

import com.epam.indigolucene.common.exceptions.CommitException;
import com.epam.indigolucene.common.exceptions.DocumentAdditionException;
import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.Query;
import com.epam.indigolucene.common.query.SolrConnection;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexRequest;
import org.elasticsearch.action.search.SearchRequestBuilder;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.transport.TransportClient;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.InetSocketTransportAddress;
import org.elasticsearch.index.query.QueryBuilders;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.transport.client.PreBuiltTransportClient;

import java.io.IOException;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.*;

/**
 * Source:      ElasticConnection.java
 * Created:     29.08.2017
 * Project:     parent-project
 * <p>
 * {@code ElasticConnection} TODO: comment
 *
 * @author Dmitrii Kuznetsov
 */
public class ElasticConnection implements SolrConnection {
    private TransportClient client;
    private Query query;
    private String url;
    private String coreName;
    private String type = "docs";

    private static final String CUT_REGEXP = "\\{[\\p{Punct}|\\w| ]*}";
    private static final String SKIP_REGEXP = "\\{[\\p{Punct}|\\w| ]*}\\w*$";

    private TransportClient getTransportClient() throws UnknownHostException, MalformedURLException {
        if (client == null) {
            synchronized (this) {
                if (client == null) {
                    URL mUrl = new URL(url);
                    client = new PreBuiltTransportClient(Settings.EMPTY)
                            .addTransportAddress(new InetSocketTransportAddress(InetAddress.getByName(mUrl.getHost()), mUrl.getPort()));
                }
            }
        }
        return client;
    }

    @Override
    public List<Map<String, Object>> results() throws Exception {
        System.out.println(query.getCondition().toJson());

        List<Map<String, Object>> results = new LinkedList<>();

        StringBuilder queryStringBuilder = new StringBuilder(query.getCondition().getSolrQ()
                .replaceAll(CUT_REGEXP, ""));
        for (String fq : query.getSolrFQs()) {
            if (fq.matches(SKIP_REGEXP))
                continue;
            if (queryStringBuilder.length() > 0)
                queryStringBuilder.append(" AND ");
            queryStringBuilder.append(fq.replaceAll(CUT_REGEXP, ""));
        }

        SearchRequestBuilder requestBuilder = getTransportClient()
                .prepareSearch(coreName)
                .setTypes(type)
                .setQuery(QueryBuilders.queryStringQuery(queryStringBuilder.toString()))
                .setSize(query.getLimit());

        SearchResponse response = requestBuilder.get();

        for (SearchHit searchHit : response.getHits().getHits()) {
            results.add(searchHit.getSourceAsMap());
        }

        return results;
    }

    @Override
    public void addDocument(Map<String, Object> fieldsMap) throws DocumentAdditionException {
        try {
            getTransportClient().prepareIndex(coreName, type).setSource(fieldsMap).get();
        } catch (UnknownHostException | MalformedURLException e) {
            throw new DocumentAdditionException(fieldsMap, e);
        }
    }

    @Override
    public void commit() throws CommitException {
        /* TODO */
    }

    @Override
    public void removeAll() throws RemoveException {
        try {
            getTransportClient().admin().indices().delete(new DeleteIndexRequest(coreName)).actionGet();
        } catch (UnknownHostException | MalformedURLException e) {
            throw new RemoveException(e);
        }
    }

    @Override
    public void close() throws IOException {
        getTransportClient().close();
        client = null; /* TODO: FIX THIS!!!*/
    }

    @Override
    public void setUrl(String url) {
        this.url = url;
    }

    @Override
    public void setCoreName(String coreName) {
        this.coreName = coreName;
    }

    @Override
    public void setQuery(Query query) {
        this.query = query;
    }
}
