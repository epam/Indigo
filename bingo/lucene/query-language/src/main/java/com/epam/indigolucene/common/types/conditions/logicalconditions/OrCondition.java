package com.epam.indigolucene.common.types.conditions.logicalconditions;

import com.epam.indigolucene.common.types.conditions.AbstractCondition;
import com.epam.indigolucene.common.types.conditions.BinaryCondition;
import com.epam.indigolucene.common.types.conditions.Condition;
import org.json.simple.JSONObject;

/**
 * Created by Artem_Malykh on 20.02.16.
 */
public class OrCondition<S> extends BinaryCondition<S> {
    public static final String OP_OR = "OR";
    public static final String SOLR_OR = "OR";

    public OrCondition(Condition<S> c1, Condition<S> c2) {
        super(c1, c2);
    }

    @Override
    public String operationName() {
        return OP_OR;
    }

    @Override
    public Condition<S> not() {
        return new AndCondition<>(getFirstOperand().not(), getSecondOperand().not());
    }

    @Override
    public String solrOperationName() {
        return SOLR_OR;
    }

    public static <S> Condition<S> orFromJson(JSONObject json) {
        JSONObject c1 = (JSONObject) json.get(BinaryCondition.FIRST_OPERAND);
        JSONObject c2 = (JSONObject) json.get(BinaryCondition.SECOND_OPERAND);

        return new OrCondition<>(AbstractCondition.fromJson(c1), AbstractCondition.fromJson(c2));
    }
}
