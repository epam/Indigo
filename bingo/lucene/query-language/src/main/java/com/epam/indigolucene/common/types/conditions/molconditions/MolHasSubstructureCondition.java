package com.epam.indigolucene.common.types.conditions.molconditions;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.ChemStructureCondition;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.utils.Utils;
import com.epam.indigolucene.common.types.conditions.FieldCondition;
import org.json.simple.JSONObject;

import java.util.LinkedList;
import java.util.List;
/**
 * Represents a condition of substructure search for molecules. Used as returning object on substructure
 * search invocation from molecule type of Solr's schema.xml class representation.
 *
 * @author Artem Malykh
 * created on 2016-02-20
 */
public class MolHasSubstructureCondition<S> extends ChemStructureCondition<S> {
    public static final String OP_SUBSTRUCTURE = "molsubs";
    public static final String QMOL         = "qMolString";

    public static int SUBSTRUCTURE_REAL_MATCH_COST = 100;
    public static int SUBSTRUCTURE_FP_MATCH_COST   = 1;

    private String qMolString;
    private IndigoObject qMol;

    public MolHasSubstructureCondition(MolField<S> field, String qMolString) {
        super(field);
        this.qMolString = qMolString;
        qMol = IndigoHolder.getIndigo().loadQueryMolecule(qMolString);
        qMol.aromatize();
    }

    public MolHasSubstructureCondition(MolField<S> field, IndigoObject qMol) {
        super(field);
        this.qMol = qMol.clone();
        this.qMol.aromatize();
        qMolString = this.qMol.cml();
    }

    @Override
    public String operationName() {
        return OP_SUBSTRUCTURE;
    }

    public static <S> MolHasSubstructureCondition<S> molHasStructureFromJson(JSONObject json) {
        String qmol =      (String) json.get(QMOL);
        String fieldName = (String) json.get(FieldCondition.FIELD_NAME);
        Boolean not = (Boolean) json.get(NOT);
        //TODO: make true serialization/deserialization for fields!!
        MolHasSubstructureCondition<S> res = new MolHasSubstructureCondition<>(new MolField<>(fieldName, false), qmol);
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
        //if this condition is negated, no filters should be applied, search should go through all documents
        /** TODO: this can be optimized in following way: we know that if any of fingerprints bits of query molecule
          * is not present in fingerprint of target molecule, this molecule has no query substructure. Let us denote
          * set of such molecules as N1. So, the result set of negated substructure query consists of two parts:
          * N1 and those molecules which have all fingerprint bits, but fail on 'true' substructure check. Let us call this
          * set N2. Finding N1 is easy: it is just query inverted fingerprint condition (pseudo-code):
          *     (NOT fingerprint_bit_1) AND (NOT fingerprint_bit_2) AND ... AND (NOT fingerprint_bit_n)
          * and N2 should be searched with regular fingerprint condition but with negated 'true' substructure check.
          * Therefore we need to somehow concatenate results of these two queries. The right way seem to add separate
          * layer in architecture for such concatenations.
         **/
        if (!not) {
            StringBuilder sb = new StringBuilder();
            sb.append("{!")
                    .append("cache=false ")
                    .append("cost=")
                    .append(SUBSTRUCTURE_FP_MATCH_COST)
                    .append("}")
                    .append(Utils.produceSolrSubsFingerprintQuery(qMol, getField().getFingerprintFieldName()));
            res.add(sb.toString());

            res.add("{!chemparsernew cost=" + SUBSTRUCTURE_REAL_MATCH_COST + "}placeholder");
        }
        return res;
    }

    @Override
    public MolField<S> getField() {
        //noinspection unchecked
        return (MolField<S>) super.getField();
    }

    @Override
    public List<ChemStructureCondition<S>> chemStructureConditions() {
        List<ChemStructureCondition<S>> res = new LinkedList<>();
        res.add(this);
        return res;
    }

    @Override
    public MolHasSubstructureCondition<S> not() {
        MolHasSubstructureCondition<S> res = new MolHasSubstructureCondition<>(getField(), qMolString);
        res.not = !this.not;
        return res;
    }

    @Override
    public boolean doMatch(IndigoObject obj) {
        return IndigoHolder.getIndigo().substructureMatcher(obj).match(qMol) != null;
    }
}

