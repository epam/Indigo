package com.epam.indigolucene.common.types.conditions;

import com.epam.indigolucene.common.types.conditions.logicalconditions.AndCondition;
import com.epam.indigolucene.common.types.conditions.logicalconditions.OrCondition;
import com.epam.indigolucene.common.types.conditions.molconditions.MolMatchesExactlyCondition;
import com.epam.indigolucene.common.types.conditions.molconditions.MolHasSubstructureCondition;
import com.epam.indigolucene.common.types.conditions.reactconditions.ReactHasSubstructureCondition;
import com.epam.indigolucene.common.types.conditions.stringconditions.StringStartsWithCondition;
import org.json.simple.JSONObject;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Predicate;

/**
 * Created by Artem Malykh on 20.02.16.
 */
public abstract class AbstractCondition<S> implements Condition<S> {
    public static final String OPERATION_NAME = "opName";
    public static final String NOT = "not";
    protected List<Predicate<Map<String, Object>>> postFilters = new LinkedList<>();
    protected boolean not;

    @Override
    public final JSONObject toJson() {
        JSONObject res = new JSONObject();
        res.put(OPERATION_NAME, operationName());
        res.put(NOT, not);
        addToJson(res);
        return res;
    }

    @Override
    public boolean isNegated() {
        return not;
    }

    private static Map<String, Function<JSONObject, Condition>> fromJsonConverters = new HashMap<>();

    static {
        fromJsonConverters.put(AndCondition.OP_AND, AndCondition::andFromJson);
        fromJsonConverters.put(OrCondition.OP_OR, OrCondition::orFromJson);
        fromJsonConverters.put(MolHasSubstructureCondition.OP_SUBSTRUCTURE, MolHasSubstructureCondition::molHasStructureFromJson);
        fromJsonConverters.put(ReactHasSubstructureCondition.OP_SUBSTRUCTURE, ReactHasSubstructureCondition::reactHasStructureFromJson);
        fromJsonConverters.put(MolMatchesExactlyCondition.OP_EXACT, MolMatchesExactlyCondition::molMatchesExactlyFromJson);
        fromJsonConverters.put(StringStartsWithCondition.OP_STARTS_WITH, StringStartsWithCondition::stringStartsWithFromJson);
    }


    public static Condition fromJson(JSONObject json) {
        String opName = (String) json.get(OPERATION_NAME);
        if (!fromJsonConverters.containsKey(opName)) {
            throw new UnsupportedOperationException();
        }
        return fromJsonConverters.get(opName).apply(json);
    }

    @Override
    public List<Predicate<Map<String, Object>>> getPostFilters() {
        return postFilters;
    }

    protected abstract void addToJson(JSONObject obj);
}
