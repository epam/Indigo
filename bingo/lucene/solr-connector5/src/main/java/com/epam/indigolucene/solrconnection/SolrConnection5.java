package com.epam.indigolucene.solrconnection;

import com.epam.indigolucene.common.exceptions.CommitException;
import com.epam.indigolucene.common.exceptions.DocumentAdditionException;
import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.Query;
import com.epam.indigolucene.common.query.SolrConnection;
import org.apache.commons.codec.binary.Base64;

import org.apache.solr.client.solrj.SolrQuery;
import org.apache.solr.client.solrj.SolrServerException;
import org.apache.solr.client.solrj.impl.HttpSolrClient;
import org.apache.solr.common.SolrDocument;
import org.apache.solr.common.SolrDocumentList;
import org.apache.solr.common.SolrInputDocument;
import org.apache.solr.common.util.ObjectReleaseTracker;

import java.io.IOException;
import java.util.*;
import java.util.function.BinaryOperator;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.stream.Collectors;


/**
 * Created by Artem Malykh on 22.03.16.
 */
public class SolrConnection5 implements SolrConnection {
    private Query query;
    private HttpSolrClient solrClient;
    private String url;
    private String coreName;

    private HttpSolrClient getHttpSolrClient() {
        if (solrClient == null) {
            synchronized (this) {
                if (solrClient == null) {
                    return new HttpSolrClient(url);
                }
            }
        }
        return solrClient;
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



    @Override
    public List<Map<String, Object>> results() throws IOException, SolrServerException {
        SolrQuery q = getRawSolrQuery();

        HttpSolrClient solrClient = getHttpSolrClient();
        //TODO: check why commented code does not work
        //SolrDocumentList results = solrClient.query(coreName, res).getResults();
        SolrDocumentList res = solrClient.query(q).getResults();
        List<Map<String, Object>> results = new LinkedList<>();

        results.addAll(res);

        Function<Map<String, Object>, Map<String, Object>> postMapper = (Function<Map<String, Object>, Map<String, Object>>) query.getCondition().getPostMappers(results).stream().reduce(Function.identity(), new BinaryOperator<Function<Map<String, Object>, Map<String, Object>>>() {
            @Override
            public Function<Map<String, Object>, Map<String, Object>> apply(Function<Map<String, Object>, Map<String, Object>> f1, Function<Map<String, Object>, Map<String, Object>> f2) {
                return f1.compose(f2);
            }
        });

        Predicate<Map<String, Object>> postFilter = (Predicate<Map<String, Object>>) query.getCondition().
                                                          getPostFilters().
                                                          stream().
                                                          reduce((Predicate<Map<String, Object>>) stringStringMap -> true, new BinaryOperator<Predicate<Map<String, Object>>>() {
                                                              @Override
                                                              public Predicate<Map<String, Object>> apply(Predicate<Map<String, Object>> mapPredicate, Predicate<Map<String, Object>> mapPredicate2) {
                                                                  return mapPredicate.and(mapPredicate2);
                                                              }
                                                          });

        return results.stream().map(postMapper).filter(postFilter).collect(Collectors.toList());
    }

    private SolrQuery getRawSolrQuery() {
        SolrQuery res = new SolrQuery();
        if (!query.getCondition().molStructureConditions().isEmpty() ||
                !query.getCondition().reactStructureConditions().isEmpty()) {
            String jq = Base64.encodeBase64String(query.toJson().toJSONString().getBytes());
            res.setParam(Query.JSON_QUERY_PARAM, jq);
            res.set("defType", "chemparsernew");
        }

        String[] fqsArray = new String[query.getSolrFQs().size()];
        for (int i = 0; i < fqsArray.length; i++) {
            fqsArray[i] = query.getSolrFQs().get(i);
        }

        if (!query.getSolrQ().isEmpty()) {
            res.setQuery(query.getSolrQ());
        }

        Map<String, String> solrParams = query.getSolrParams();
        for (String s : solrParams.keySet()) {
            res.setParam(s, solrParams.get(s));
        }

        if (fqsArray.length > 0) {
            res.setFilterQueries(fqsArray);
        }

        Set<String> fields = query.getIncludeFields();
        fields.addAll(query.getCondition().getAdditionalFields());

        res.setFields(fields.toArray(new String[fields.size()]));
        res.setStart(query.getOffset());
        res.setRows(query.getLimit());
        res.set("limit", query.getLimit() + "");
        return res;


    }

    private void addDocumentInternal(HttpSolrClient client, String coreName, Map<String, Object> fieldsMap, boolean commitNow) throws IOException, SolrServerException {
        SolrInputDocument res = new SolrInputDocument();

        for (Map.Entry<String, Object> kv : fieldsMap.entrySet()) {
            String fieldName  = kv.getKey();
            Object fieldValue = kv.getValue();

            res.addField(fieldName, fieldValue);
        }

        //TODO: check why commented code does not work
        //client.add(coreName, res);
        client.add(res);
        if (commitNow) {
            client.commit();
        }
    }

    @Override
    public void addDocument(Map<String, Object> fieldsMap) throws DocumentAdditionException {
        HttpSolrClient solrClient = new HttpSolrClient(url);
        try {
            addDocumentInternal(solrClient, coreName, fieldsMap, false);
        } catch (Exception e) {
            throw new DocumentAdditionException(fieldsMap, e);
        }
    }

    @Override
    public void commit() throws CommitException {
        try {
            getHttpSolrClient().commit();
        } catch (Exception e) {
            throw new CommitException(e);
        }
    }

    @Override
    public void removeAll() throws RemoveException {
        try {
            getHttpSolrClient().deleteByQuery("*:*");
        } catch (Exception e) {
            throw new RemoveException(e);
        }
    }


    @Override
    public void close() throws IOException {
        getHttpSolrClient().close();
    }
}
