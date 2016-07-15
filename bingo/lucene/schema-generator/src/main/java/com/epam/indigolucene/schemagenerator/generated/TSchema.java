
package com.epam.indigolucene.schemagenerator.generated;

import java.util.HashSet;
import java.util.Set;
import javax.annotation.Generated;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.types.fields.StringField;
import com.epam.indigolucene.common.types.fields.searchable.SearchableMolField;
import com.epam.indigolucene.common.types.fields.searchable.SearchableStringField;

@Generated("com.epam.indigolucene.schemagenerator.JavaSolrSchemaRepresentationGenerator")
public class TSchema {

    public final static StringField<TSchema> MONGO_ID = new StringField<TSchema>("mongoId", false);
    public final static StringField<TSchema> SIGMA_SYNONYMS = new StringField<TSchema>("sigma_synonyms", true);
    public final static StringField<TSchema> NEGWER_SYNONYMS = new StringField<TSchema>("negwer_synonyms", true);
    public final static SearchableStringField<TSchema> DOC_ID = new SearchableStringField<TSchema>("docId", false);
    public final static SearchableStringField<TSchema> MOL_FINGERPRINT = new SearchableStringField<TSchema>("mol_fingerprint", true);
    public final static SearchableMolField<TSchema> MOL = new SearchableMolField<TSchema>("mol", false);
    public final static StringField<TSchema> CAS_NUMBER = new StringField<TSchema>("casNumber", true);
    public final static StringField<TSchema> MOL_ID = new StringField<TSchema>("molId", false);
    public final static StringField<TSchema> MERCK_SYNONYMS = new StringField<TSchema>("merck_synonyms", true);
    public final static StringField<TSchema> BASE_SYNONYMS = new StringField<TSchema>("base_synonyms", true);
    public final static StringField<TSchema> DOC_URL = new StringField<TSchema>("docUrl", false);
    public final static StringField<TSchema> CAS_SYNONYMS = new StringField<TSchema>("cas_synonyms", true);
    public final static StringField<TSchema> SIGMA_ALDRICH_SYNONYMS = new StringField<TSchema>("sigmaAldrich_synonyms", true);
    public final static StringField<TSchema> NEGWER_NUMBER = new StringField<TSchema>("negwerNumber", false);
    public final static StringField<TSchema> ID = new StringField<TSchema>("id", false);
    public final static StringField<TSchema> DEFAULT_NAME = new StringField<TSchema>("defaultName", false);
    public final static StringField<TSchema> CONTENT_TYPE = new StringField<TSchema>("contentType", false);
    public final static StringField<TSchema> _TEXT_ = new StringField<TSchema>("_text_", true);
    private static Set<String> fieldNames = new HashSet();

    static {
        fieldNames.add("mongoId");
        fieldNames.add("sigma_synonyms");
        fieldNames.add("negwer_synonyms");
        fieldNames.add("docId");
        fieldNames.add("mol_fingerprint");
        fieldNames.add("mol");
        fieldNames.add("casNumber");
        fieldNames.add("molId");
        fieldNames.add("merck_synonyms");
        fieldNames.add("base_synonyms");
        fieldNames.add("docUrl");
        fieldNames.add("cas_synonyms");
        fieldNames.add("sigmaAldrich_synonyms");
        fieldNames.add("negwerNumber");
        fieldNames.add("id");
        fieldNames.add("defaultName");
        fieldNames.add("contentType");
        fieldNames.add("_text_");
    }

    public static CollectionRepresentation<TSchema> collection(String url, String coreName) {
        return new CollectionRepresentation<TSchema>(url, coreName, fieldNames);
    }

    public static TSchemaDocument createEmptyDocument() {
        return new TSchemaDocument();
    }

}
