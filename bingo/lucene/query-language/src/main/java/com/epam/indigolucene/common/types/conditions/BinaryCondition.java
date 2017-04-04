package com.epam.indigolucene.common.types.conditions;

import org.json.simple.JSONObject;

import java.util.LinkedList;
import java.util.List;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * Created by Artem Malykh on 20.02.16.
 */
public abstract class BinaryCondition<S> extends AbstractCondition<S> {
    public static String FIRST_OPERAND = "op1";
    public static String SECOND_OPERAND = "op2";

    private Condition<S> operand1;
    private Condition<S> operand2;

    public abstract String solrOperationName();

    public BinaryCondition(Condition<S> firstOperand, Condition<S> secondOperand) {
        operand1 = firstOperand;
        operand2 = secondOperand;
    }

    @Override
    public List<ChemStructureCondition<S>> chemStructureConditions() {
        List<ChemStructureCondition<S>> res = new LinkedList<>();
        res.addAll(operand1.chemStructureConditions());
        res.addAll(operand2.chemStructureConditions());
        return res;
    }

    @Override
    protected final void addToJson(JSONObject obj) {
        obj.put(FIRST_OPERAND, getFirstOperand().toJson());
        obj.put(SECOND_OPERAND, getSecondOperand().toJson());
    }

    @Override
    public String getSolrQ() {
        List<String> operandsQueries = new LinkedList<>();
        operandsQueries.add(operand1.getSolrQ());
        operandsQueries.add(operand2.getSolrQ());

        List<String> bracketed = operandsQueries.stream().filter(oq -> oq != null && !oq.isEmpty())
                .map(oq -> "(" + oq + ")").collect(Collectors.toList());
        return String.join(solrOperationName(), bracketed);
    }

    @Override
    public List<String> getSolrFQs() {
        LinkedList<String> res = new LinkedList<>(operand1.getSolrFQs());
        res.addAll(operand2.getSolrFQs());
        return res;
    }

    protected static final <T> T binaryOperatorFromJson(JSONObject obj, Function<JSONObject, T> converter, BiFunction<T, T, T> combiner) {
        JSONObject operand1 = (JSONObject) obj.get(FIRST_OPERAND);
        JSONObject operand2 = (JSONObject) obj.get(SECOND_OPERAND);

        return combiner.apply(converter.apply(operand1), converter.apply(operand2));
    }

    public final Condition<S> getFirstOperand() {
        return operand1;
    }
    public final Condition<S> getSecondOperand() {
        return operand2;
    }
}
