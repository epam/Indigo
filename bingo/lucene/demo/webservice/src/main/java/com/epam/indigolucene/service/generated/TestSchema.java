
package com.epam.indigolucene.service.generated;

import java.util.HashSet;
import java.util.Set;
import javax.annotation.Generated;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.types.fields.StringField;
import com.epam.indigolucene.common.types.fields.searchable.SearchableMolField;

@Generated("com.epam.indigolucene.schemagenerator.JavaSolrSchemaRepresentationGenerator")
public class TestSchema {

    public final static StringField<TestSchema> MOL_ID = new StringField<TestSchema>("molId", false);
    public final static SearchableMolField<TestSchema> MOL = new SearchableMolField<TestSchema>("mol", false);
    private static Set<String> fieldNames = new HashSet();

    static {
        fieldNames.add("molId");
        fieldNames.add("mol");
    }

    public static CollectionRepresentation<TestSchema> collection(String url, String coreName) {
        return new CollectionRepresentation<TestSchema>(url, coreName, fieldNames);
    }

    public static TestSchemaDocument createEmptyDocument() {
        return new TestSchemaDocument();
    }

}
