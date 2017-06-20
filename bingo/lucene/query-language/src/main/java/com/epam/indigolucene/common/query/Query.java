package com.epam.indigolucene.common.query;

import com.epam.indigolucene.common.types.conditions.AbstractCondition;
import com.epam.indigolucene.common.types.conditions.Condition;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Query class is a simple query to a Solr-server representation with a few additional JSON-conversion methods.
 *
 * @author Artem Malykh
 * created on 2016-02-24
 */
public class Query {
    public static String JSON_QUERY_PARAM = "chemQueryJson";
    public static String CONDITION = "cond";
    public static String FIELDS = "flds";
    public static String OFFSET = "ofst";
    public static String LIMIT = "lmt";

    public static int DEFAULT_LIMIT = 10;

    Condition condition;

    Set<String> dropField;
    Set<String> includeField;

    Set<String> resultFields;

    int offset;
    int limit;

    protected boolean firstInclude;

    Query(Set<String> fields) throws InstantiationException, IllegalAccessException {
        dropField    = new HashSet<>();
        includeField = new HashSet<>(fields);
        offset       = 0;
        limit        = DEFAULT_LIMIT;
        firstInclude = true;
    }

    Query(Set<String> resultFields, int offset, int limit, Condition condition) {
        this.resultFields = resultFields;
        this.offset = offset;
        this.limit = limit;
        this.condition = condition;
    }

    @SuppressWarnings("unchecked")
    public JSONObject toJson() {
        //We turn query into json and then
        JSONObject res = new JSONObject();

        if (condition != null) {
            res.put(CONDITION, condition.toJson());
        }
        res.put(OFFSET, offset);
        if (limit >= 0) {
            res.put(LIMIT, limit);
        }

        JSONArray fields = new JSONArray();
        Set<String> iFields = new HashSet<>(includeField);
        iFields.removeAll(dropField);
        fields.addAll(iFields);

        res.put(FIELDS, fields);

        return res;
    }


    public static Query fromJson(JSONObject json) {
        return new Query((Set<String>)((JSONArray)json.get(FIELDS)).stream().collect(Collectors.toSet()),
                Integer.valueOf(json.get(OFFSET).toString()),
                Integer.valueOf(json.get(LIMIT).toString()),
                AbstractCondition.fromJson((JSONObject) json.get(CONDITION)));//
    }

    public List<String> getSolrFQs() {
        return condition.getSolrFQs();
    }

    public String getSolrQ() {
        return condition.getSolrQ();
    }

    public Map<String, String> getSolrParams() {
        return condition.getSolrParams();
    }

    public Condition getCondition() {
        return condition;
    }

    public int getOffset() {
        return offset;
    }

    public int getLimit() {
        return limit;
    }

    public Set<String> getIncludeFields() {
        //TODO: maybe should do defensive copy
        return includeField;
    }
}
