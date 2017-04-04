package com.epam.indigolucene.common.types.conditions;

import com.epam.indigolucene.common.types.conditions.logicalconditions.AndCondition;
import com.epam.indigolucene.common.types.conditions.logicalconditions.OrCondition;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.function.Function;
import java.util.function.Predicate;

/**
 * This class encapsulates condition used in queries to solr.
 * Created by Artem Malykh on 12.04.16.
 */
public interface Condition<S> {
    String operationName();

    /**
     * Get solr query ("q" parameter) to which this condition transforms.
     * @return solr query.
     */
    default String getSolrQ() {
        return "";
    }

    default Map<String, String> getSolrParams() {
        return new HashMap<>();
    }

    /**
     * Get solr filter queries ("fq" array) to which this condition transforms.
     * @return solr filter queries array.
     */
    default List<String> getSolrFQs() {
        return new LinkedList<>();
    }

    /**
     * Get all conditions which should be considered on the post-filter stage (these are "expensive" parts of molecules matching)
     * @return
     */
    default List<ChemStructureCondition<S>> chemStructureConditions() {
        return new LinkedList<>();
    }

    /** TODO: Difficult to implement fully functional logical 'or'. Reason:
     * Currently on solr side query is executed in two steps:
     * 1. Regular solr query (if there is mol field in original query, solr query will contain condition on fingerprints)
     * 2. Post filtering. If there is original query, elements which come as input to this step can be considered as only candidates
     * (since fingerprint match does not equal to real match between two molecules. In post filtering we filter out such of candidates
     * that does not "real" match. So for each candidate we "kick" it if it does not "real" match. But if we have some_condition OR condition_on_molecule
     * even if molecule does not "real" match query molecule, the candidate can be still valid if some_condition is TRUE. I don't know
     * currently how to obtain information about boolean value of original query on candidates if they do not "real" match.
     * Maybe it can be done in two queries: first is query with FALSE substituted instead of molecule_condition. And second is
     * original query. Then the result is union of results of these queries.
     **/
    @Deprecated
    default OrCondition<S> or(Condition c) {
        return new OrCondition<>(this, c);
    }

    /**
     * Creates new condition which is logical 'and' between this  condition and c
     * @param c some condition
     * @return condition which is logical 'and' between this  condition and c
     */
    default AndCondition<S> and(Condition c) {
        return new AndCondition<>(this, c);
    }

    /**
     * Negate current condition
     * @return negated condition
     */
    Condition<S> not();

    /**
     * Returns negated condition
     * @return negated condition
     */
    boolean isNegated();

    JSONObject toJson();

    List<Predicate<Map<String, Object>>> getPostFilters();

    default List<Function<Map<String, Object>, Map<String, Object>>> getPostMappers(List<Map<String, Object>> res) {
        return new LinkedList<>();
    }

    default Set<String> getAdditionalFields() {return new HashSet<>();}

}
