package com.epam.indigolucene.common.types.conditions.similarityconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.conditions.PostFilterable;
import com.epam.indigolucene.common.types.fields.ChemField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.function.Function;

/**
 * This class encapsulates condition of similarity between molecules.
 * Created by Artem Malykh on 12.04.16.
 */
public class ChemSimilarTo<S> extends FieldCondition<S> implements PostFilterable<S, Float> {
    public static final String OP_CHEMICAL_SIMILAR = "chem_sim";
    public static final String Q_FINGERPRINT = "qFp";

    private String qFp;
    private IndigoObject qChem;

    public ChemSimilarTo(ChemField<S> field, String qChemString) {
        super(field);
        //If there is a reaction sign in a chemical string, then think of chemical as reaction
        if(qChemString.contains(">")) {
            qChem = IndigoHolder.getIndigo().loadReaction(qChemString);
        } else {
            //otherwise as molecule
           qChem = IndigoHolder.getIndigo().loadQueryMolecule(qChemString);
        }
        qChem.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(qChem, getField().getFingerPrintFieldName());
    }

    public ChemSimilarTo(ChemField<S> field, IndigoObject qChem) {
        super(field);
        this.qChem = qChem.clone();
        this.qChem.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(this.qChem, getField().getFingerPrintFieldName());
    }

    @Override
    public String operationName() {
        return OP_CHEMICAL_SIMILAR;
    }

    @Override
    public ChemField<S> getField() {
        return (ChemField<S>) super.getField();
    }

    @Override
    public String getSolrQ() {
        return qFp;
    }

    @Override
    public Condition<S> not() {
        ChemSimilarTo<S> res = new ChemSimilarTo<>(getField(), qChem);
        res.not = !this.not;
        res.qFp = Utils.produceSolrSimilarityQuery(res.qChem, res.getField().getFingerPrintFieldName(), res.not);
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
            final float referenceSimilarity = IndigoHolder.getIndigo().similarity(qChem, topResult);
            final float topScore = getValue(r.get(0));
            res.add(stringStringMap -> {
                stringStringMap.put("score", getValue(stringStringMap) / topScore * referenceSimilarity);
                return stringStringMap;
            });
        }
        return res;
    }
}
