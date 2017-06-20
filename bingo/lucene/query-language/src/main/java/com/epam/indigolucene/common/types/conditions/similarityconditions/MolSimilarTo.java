package com.epam.indigolucene.common.types.conditions.similarityconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.conditions.PostFilterable;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.*;
import java.util.function.Function;
/**
 * Represents a condition of similarity search for molecules. Used as returning object on similarity
 * search invocation from molecule type of Solr's schema.xml class representation.
 *
 * @author Artem Malykh
 * created on 2016-04-12
 */
public class MolSimilarTo<S> extends FieldCondition<S> implements PostFilterable<S, Float> {
    public static final String OP_MOLECULE_SIMILAR = "mol_sim";
    public static final String Q_FINGERPRINT = "qFp";

    private String qFp;
    private IndigoObject qMol;

    public MolSimilarTo(MolField<S> field, String qMolString) {
        super(field);
        qMol = IndigoHolder.getIndigo().loadMolecule(qMolString);
        qMol.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(qMol, getField().getFingerprintFieldName());
    }

    public MolSimilarTo(MolField<S> field, IndigoObject qMol) {
        super(field);
        this.qMol = qMol.clone();
        this.qMol.aromatize();
        qFp = Utils.produceSolrSimilarityQuery(this.qMol, getField().getFingerprintFieldName());
    }

    @Override
    public String operationName() {
        return OP_MOLECULE_SIMILAR;
    }

    @Override
    public MolField<S> getField() {
        return (MolField<S>) super.getField();
    }

    @Override
    public String getSolrQ() {
        return qFp;
    }

    @Override
    public Condition<S> not() {
        MolSimilarTo<S> res = new MolSimilarTo<>(getField(), qMol);
        res.not = !this.not;
        res.qFp = Utils.produceSolrSimilarityQuery(res.qMol, res.getField().getFingerprintFieldName(), res.not);
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
            final float referenceSimilarity = IndigoHolder.getIndigo().similarity(qMol, topResult);
            final float topScore = getValue(r.get(0));
            res.add(stringStringMap -> {
                stringStringMap.put("score", getValue(stringStringMap) / topScore * referenceSimilarity);
                return stringStringMap;
            });
        }
        return res;
    }
}
