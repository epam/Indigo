package com.epam.indigolucene.common.types.conditions.molconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Created by Artem Malykh on 03.03.16.
 */
public class MolMatchesExactlyCondition<S> extends MolStructureCondition<S> {
    public static final String OP_EXACT = "exact";
    public static final String QMOL         = "qMolString";
    public static float SUBSTRUCTURE_MATCHES = 1.0f;

    public static int SUBSTRUCTURE_REAL_MATCH_COST = 100;
    public static int SUBSTRUCTURE_FP_MATCH_COST   = 1;

    private String qMolString;
    private IndigoObject qMol;

    public MolMatchesExactlyCondition(MolField<S> field, String qMolString) {
        super(field);
        this.qMolString = qMolString;
        qMol = IndigoHolder.getIndigo().loadMolecule(qMolString);
        qMol.aromatize();
    }

    public MolMatchesExactlyCondition(MolField<S> field, IndigoObject qMol) {
        super(field);
        this.qMol = qMol.clone();
        this.qMol.aromatize();
        qMolString = this.qMol.cml();
    }

    @Override
    public String operationName() {
        return OP_EXACT;
    }

    public static <S> MolMatchesExactlyCondition<S> molMatchesExactlyFromJson(JSONObject json) {
        String qmol =      (String) json.get(QMOL);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        Boolean not = (Boolean) json.get(FieldCondition.NOT);
        //TODO: make true serialization/deserialization for fields!!
        MolMatchesExactlyCondition<S> res = new MolMatchesExactlyCondition<>(new MolField<>(fieldName, false), qmol);
        res.not = not;
        return res;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        obj.put(QMOL, qMolString);
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
                    .append(Utils.produceSolrSubsFingerprintQuery(qMol, getField().getFingerprintFieldName()));
            res.add(sb.toString());
        }

        res.add("{!chemparsernew cost=" + SUBSTRUCTURE_REAL_MATCH_COST + "}placeholder");
        return res;
    }

    @Override
    public MolField<S> getField() {
        //noinspection unchecked
        return (MolField<S>) super.getField();
    }

    @Override
    public List<MolStructureCondition<S>> molStructureConditions() {
        List<MolStructureCondition<S>> res = new LinkedList<>();
        res.add(this);
        return res;
    }

    @Override
    public Condition<S> not() {
        MolMatchesExactlyCondition<S> res = new MolMatchesExactlyCondition<>(getField(), qMolString);
        res.not = !this.not;
        return res;
    }

    @Override
    public boolean doMatch(IndigoObject obj) {
        return IndigoHolder.getIndigo().exactMatch(qMol, obj) != null;
    }
}