
package indexer.data.generated;

import java.util.HashSet;
import java.util.Set;
import javax.annotation.Generated;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.types.fields.StringField;
import com.epam.indigolucene.common.types.fields.searchable.SearchableChemField;
import com.epam.indigolucene.common.types.fields.searchable.SearchableStringField;

@Generated("com.epam.indigolucene.schemagenerator.JavaSolrSchemaRepresentationGenerator")
public class TestSchema {

    public final static StringField<TestSchema> CHEM_ID = new StringField<TestSchema>("chemId", false);
    public final static SearchableChemField<TestSchema> CHEM = new SearchableChemField<TestSchema>("chem", false);
    public final static SearchableStringField<TestSchema> DOC_ID = new SearchableStringField<TestSchema>("docId", false);
    public final static StringField<TestSchema> TEST_MULTI_VAL = new StringField<TestSchema>("testMultiVal", false);
    public final static StringField<TestSchema> DOC_URL = new StringField<TestSchema>("docUrl", false);
    public final static SearchableStringField<TestSchema> CONTENT_TYPE = new SearchableStringField<TestSchema>("contentType", false);
    private static Set<String> fieldNames = new HashSet();

    static {
        fieldNames.add("chemId");
        fieldNames.add("chem");
        fieldNames.add("docId");
        fieldNames.add("testMultiVal");
        fieldNames.add("docUrl");
        fieldNames.add("contentType");
    }

    public static CollectionRepresentation<TestSchema> collection(String url, String coreName) {
        return new CollectionRepresentation<TestSchema>(url, coreName, fieldNames);
    }

    public static TestSchemaDocument createEmptyDocument() {
        return new TestSchemaDocument();
    }

}
