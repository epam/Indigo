package com.epam.indigolucene.common.types.conditions.chemconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.ChemStructureCondition;
import com.epam.indigolucene.common.types.conditions.Condition;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import com.epam.indigolucene.common.types.fields.ChemField;
import com.epam.indigolucene.common.utils.Utils;
import org.json.simple.JSONObject;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by Artem Malykh on 03.03.16.
 */
public class ChemMatchesExactlyCondition<S> extends ChemStructureCondition<S> {
    public static final String OP_EXACT = "chemexact";
    public static final String QCHEM         = "qChemString";
    public static float SUBSTRUCTURE_MATCHES = 1.0f;

    public static int SUBSTRUCTURE_REAL_MATCH_COST = 100;
    public static int SUBSTRUCTURE_FP_MATCH_COST   = 1;

    private String qChemString;
    private IndigoObject qChem;

    public ChemMatchesExactlyCondition(ChemField<S> field, String qChemString) {
        super(field);
        this.qChemString = qChemString;
        //If there is a reaction sign in a chemical string, then think of chemical as reaction
        if(qChemString.contains(">")) {
            qChem = IndigoHolder.getIndigo().loadReaction(qChemString);
        } else {
            //otherwise as molecule
            qChem = IndigoHolder.getIndigo().loadQueryMolecule(qChemString);
        }
        qChem.aromatize();
    }

    public ChemMatchesExactlyCondition(ChemField<S> field, IndigoObject qChem) {
        super(field);
        this.qChem = qChem.clone();
        this.qChem.aromatize();
        qChemString = this.qChem.cml();
    }

    @Override
    public String operationName() {
        return OP_EXACT;
    }

    public static <S> ChemMatchesExactlyCondition<S> chemMatchesExactlyFromJson(JSONObject json) {
        String qchem =      (String) json.get(QCHEM);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        Boolean not = (Boolean) json.get(FieldCondition.NOT);
        //TODO: make true serialization/deserialization for fields!!
        ChemMatchesExactlyCondition<S> res = new ChemMatchesExactlyCondition<>(new ChemField<>(fieldName, false), qchem);
        res.not = not;
        return res;
    }

    @Override
    protected void addFieldConditionDataToJson(JSONObject obj) {
        obj.put(QCHEM, qChemString);
    }

    @Override
    public List<String> getSolrFQs() {
        List<String> res = new LinkedList<>();

        //TODO: optimization needed here, see comment in same method in 'ChemHasSubstructureCondition'
        if (!not) {
            StringBuilder sb = new StringBuilder();
            sb.append("{!")
                    .append("cache=false ")
                    .append("cost=")
                    .append(SUBSTRUCTURE_FP_MATCH_COST)
                    .append("}")
                    .append(Utils.produceSolrSubsFingerprintQuery(qChem, getField().getFingerPrintFieldName()));
            res.add(sb.toString());
        }

        res.add("{!chemparsernew cost=" + SUBSTRUCTURE_REAL_MATCH_COST + "}placeholder");
        return res;
    }

    @Override
    public ChemField<S> getField() {
        //noinspection unchecked
        return (ChemField<S>) super.getField();
    }

    @Override
    public List<ChemStructureCondition<S>> chemStructureConditions() {
        List<ChemStructureCondition<S>> res = new LinkedList<>();
        res.add(this);
        return res;
    }

    @Override
    public Condition<S> not() {
        ChemMatchesExactlyCondition<S> res = new ChemMatchesExactlyCondition<>(getField(), qChemString);
        res.not = !this.not;
        return res;
    }

    @Override
    public boolean doMatch(IndigoObject obj) {
        return IndigoHolder.getIndigo().exactMatch(qChem, obj) != null;
    }
}