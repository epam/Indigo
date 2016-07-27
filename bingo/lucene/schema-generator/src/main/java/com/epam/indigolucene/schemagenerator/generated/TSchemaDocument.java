
package com.epam.indigolucene.schemagenerator.generated;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.DocumentRepresentation;
import com.epam.indigolucene.common.types.values.MolValue;
import com.epam.indigolucene.common.types.values.StringValue;
import com.epam.indigolucene.common.utils.Utils;
import com.epam.indigolucene.schemagenerator.generated.TSchema;

public class TSchemaDocument
    implements DocumentRepresentation<TSchema>
{

    private StringValue<TSchema> mongoId;
    private StringValue<TSchema> sigma_synonyms;
    private StringValue<TSchema> negwer_synonyms;
    private StringValue<TSchema> docId;
    private StringValue<TSchema> mol_fingerprint;
    private MolValue<TSchema> mol;
    private StringValue<TSchema> casNumber;
    private StringValue<TSchema> molId;
    private StringValue<TSchema> merck_synonyms;
    private StringValue<TSchema> base_synonyms;
    private StringValue<TSchema> docUrl;
    private StringValue<TSchema> cas_synonyms;
    private StringValue<TSchema> sigmaAldrich_synonyms;
    private StringValue<TSchema> negwerNumber;
    private StringValue<TSchema> id;
    private StringValue<TSchema> defaultName;
    private StringValue<TSchema> contentType;
    private StringValue<TSchema> _text_;

    public TSchemaDocument() {
    }

    @Override
    public Map<String, Object> fieldsMap() {
        Map<String, Object> res = new HashMap();
        Utils.addValueToFieldsMap(mongoId, res);
        Utils.addValueToFieldsMap(sigma_synonyms, res);
        Utils.addValueToFieldsMap(negwer_synonyms, res);
        Utils.addValueToFieldsMap(docId, res);
        Utils.addValueToFieldsMap(mol_fingerprint, res);
        Utils.addValueToFieldsMap(mol, res);
        Utils.addValueToFieldsMap(casNumber, res);
        Utils.addValueToFieldsMap(molId, res);
        Utils.addValueToFieldsMap(merck_synonyms, res);
        Utils.addValueToFieldsMap(base_synonyms, res);
        Utils.addValueToFieldsMap(docUrl, res);
        Utils.addValueToFieldsMap(cas_synonyms, res);
        Utils.addValueToFieldsMap(sigmaAldrich_synonyms, res);
        Utils.addValueToFieldsMap(negwerNumber, res);
        Utils.addValueToFieldsMap(id, res);
        Utils.addValueToFieldsMap(defaultName, res);
        Utils.addValueToFieldsMap(contentType, res);
        Utils.addValueToFieldsMap(_text_, res);
        return res;
    }

    public TSchemaDocument setMongoId(String val) {
        this.mongoId = TSchema.MONGO_ID.createValue(val);
        return this;
    }

    public TSchemaDocument setSigma_synonyms(List<String> val) {
        this.sigma_synonyms = TSchema.SIGMA_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setNegwer_synonyms(List<String> val) {
        this.negwer_synonyms = TSchema.NEGWER_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setDocId(String val) {
        this.docId = TSchema.DOC_ID.createValue(val);
        return this;
    }

    public TSchemaDocument setMol_fingerprint(List<String> val) {
        this.mol_fingerprint = TSchema.MOL_FINGERPRINT.createValue(val);
        return this;
    }

    public TSchemaDocument setMol(IndigoObject val) {
        this.mol = TSchema.MOL.createValue(val);
        return this;
    }

    public TSchemaDocument setCasNumber(List<String> val) {
        this.casNumber = TSchema.CAS_NUMBER.createValue(val);
        return this;
    }

    public TSchemaDocument setMolId(String val) {
        this.molId = TSchema.MOL_ID.createValue(val);
        return this;
    }

    public TSchemaDocument setMerck_synonyms(List<String> val) {
        this.merck_synonyms = TSchema.MERCK_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setBase_synonyms(List<String> val) {
        this.base_synonyms = TSchema.BASE_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setDocUrl(String val) {
        this.docUrl = TSchema.DOC_URL.createValue(val);
        return this;
    }

    public TSchemaDocument setCas_synonyms(List<String> val) {
        this.cas_synonyms = TSchema.CAS_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setSigmaAldrich_synonyms(List<String> val) {
        this.sigmaAldrich_synonyms = TSchema.SIGMA_ALDRICH_SYNONYMS.createValue(val);
        return this;
    }

    public TSchemaDocument setNegwerNumber(String val) {
        this.negwerNumber = TSchema.NEGWER_NUMBER.createValue(val);
        return this;
    }

    public TSchemaDocument setId(String val) {
        this.id = TSchema.ID.createValue(val);
        return this;
    }

    public TSchemaDocument setDefaultName(String val) {
        this.defaultName = TSchema.DEFAULT_NAME.createValue(val);
        return this;
    }

    public TSchemaDocument setContentType(String val) {
        this.contentType = TSchema.CONTENT_TYPE.createValue(val);
        return this;
    }

    public TSchemaDocument set_text_(List<String> val) {
        this._text_ = TSchema._TEXT_.createValue(val);
        return this;
    }

}
