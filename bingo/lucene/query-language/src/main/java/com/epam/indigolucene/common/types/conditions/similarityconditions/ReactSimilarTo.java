package com.epam.indigolucene.common.types.conditions.similarityconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.conditions.PostFilterable;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.function.Function;
/**
 * Represents a condition of similarity search for reactions. Used as returning object on similarity
 * search invocation from reaction type of Solr's schema.xml class representation.
 *
 * @author Filipp Pisarev
 * created on 01/06/2017
 */
public class ReactSimilarTo<S> extends FieldCondition<S> implements PostFilterable<S, Float> {
    public static final String OP_REACTION_SIMILAR = "react_sim";
    public static final String Q_FINGERPRINT = "qFp";

    private String qFp;
    private IndigoObject qReact;

    public ReactSimilarTo(ReactField<S> field, String qReactString) {
        super(field);
        qReact = IndigoHolder.getIndigo().loadReaction(qReactString);
        qReact.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(qReact, getField().getFingerPrintFieldName());
    }

    public ReactSimilarTo(ReactField<S> field, IndigoObject qReact) {
        super(field);
        this.qReact = qReact.clone();
        this.qReact.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(this.qReact, getField().getFingerPrintFieldName());
    }

    @Override
    public String operationName() {
        return OP_REACTION_SIMILAR;
    }

    @Override
    public ReactField<S> getField() {
        return (ReactField<S>) super.getField();
    }

    @Override
    public String getSolrQ() {
        return qFp;
    }

    @Override
    public Condition<S> not() {
        ReactSimilarTo<S> res = new ReactSimilarTo<>(getField(), qReact);
        res.not = !this.not;
        res.qFp = Utils.produceSolrSimilarityQuery(res.qReact, res.getField().getFingerPrintFieldName(), res.not);
        return res;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        //noinspection unchecked
        obj.put(Q_FINGERPRINT, qFp);
    }

    @Override
    public Set<String> getAdditionalFields() {
        Set<String> res = new HashSet<>();
        res.add("score");
        return res;
    }

    @Override
    public Float getValue(Map<String, Object> res) {
        return (Float) res.get("score");
    }

    @Override
    public List<Function<Map<String, Object>, Map<String, Object>>> getPostMappers(List<Map<String, Object>> r) {
        List<Function<Map<String, Object>, Map<String, Object>>> res = new LinkedList<>();
        if (r.size() > 0) {
            IndigoObject topResult = IndigoHolder.getIndigo().unserialize((byte[]) r.get(0).get(getFieldName()));
            final float referenceSimilarity = IndigoHolder.getIndigo().similarity(qReact, topResult);
            final float topScore = getValue(r.get(0));
            res.add(stringStringMap -> {
                stringStringMap.put("score", getValue(stringStringMap) / topScore * referenceSimilarity);
                return stringStringMap;
            });
        }
        return res;
    }

}
