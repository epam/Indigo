package com.epam.indigolucene.common.types.conditions.reactconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.ChemStructureCondition;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.LinkedList;
import java.util.List;
/**
 * Represents a condition of exact search for reactions. Used as returning object on exact
 * search invocation from reaction type of Solr's schema.xml class representation.
 *
 * @author Filipp Pisarev
 * created on 2017-04-07
 */
public class ReactMatchesExactlyCondition<S> extends ChemStructureCondition<S>{
    public static final String OP_EXACT = "reactexact";
    public static final String QREACT = "qReactString";
    public static float SUBSTRUCTURE_MATCHES = 1.0f;

    public static int SUBSTRUCTURE_REAL_MATCH_COST = 100;
    public static int SUBSTRUCTURE_FP_MATCH_COST   = 1;

    private String qReactString;
    private IndigoObject qReact;

    public ReactMatchesExactlyCondition(ReactField<S> field, String qReactString) {
        super(field);
        this.qReactString = qReactString;
        qReact = IndigoHolder.getIndigo().loadReaction(qReactString);
        qReact.aromatize();
    }

    public ReactMatchesExactlyCondition(ReactField<S> field, IndigoObject qReact) {
        super(field);
        this.qReact = qReact.clone();
        this.qReact.aromatize();
        qReactString = this.qReact.cml();
    }

    @Override
    public String operationName() {
        return OP_EXACT;
    }

    public static <S> ReactMatchesExactlyCondition reactMatchesExactlyFromJson(JSONObject json) {
        String qreact =      (String) json.get(QREACT);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        Boolean not = (Boolean) json.get(FieldCondition.NOT);
        ReactMatchesExactlyCondition<S> res = new ReactMatchesExactlyCondition<>(new ReactField<>(fieldName, false), qreact);
        res.not = not;
        return res;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        obj.put(QREACT, qReactString);
    }

    @Override
    public List<String> getSolrFQs() {
        List<String> res = new LinkedList<>();

        //TODO: optimization needed here, see comment in same method in 'MolHasSubstructureCondition'
        if (!not) {
            StringBuilder sb = new StringBuilder();
            sb.append("{!")
                    .append("cache=false ")
                    .append("cost=")
                    .append(SUBSTRUCTURE_FP_MATCH_COST)
                    .append("}")
                    .append(Utils.produceSolrSubsFingerprintQuery(qReact, getField().getFingerPrintFieldName()));
            res.add(sb.toString());
        }

        res.add("{!chemparsernew cost=" + SUBSTRUCTURE_REAL_MATCH_COST + "}placeholder");
        return res;
    }

    @Override
    public ReactField<S> getField() {
        //noinspection unchecked
        return (ReactField<S>) super.getField();
    }

    @Override
    public List<ChemStructureCondition<S>> chemStructureConditions() {
        List<ChemStructureCondition<S>> res = new LinkedList<>();
        res.add(this);
        return res;
    }

    @Override
    public Condition<S> not() {
        ReactMatchesExactlyCondition<S> res = new ReactMatchesExactlyCondition<>(getField(), qReactString);
        res.not = !this.not;
        return res;
    }

    @Override
    public boolean doMatch(IndigoObject obj) {
        return IndigoHolder.getIndigo().exactMatch(qReact, obj) != null;
    }
}
