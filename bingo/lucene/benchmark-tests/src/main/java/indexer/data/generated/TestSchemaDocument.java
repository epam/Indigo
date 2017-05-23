
package indexer.data.generated;

import java.util.HashMap;
import java.util.Map;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.DocumentRepresentation;
import com.epam.indigolucene.common.types.values.MolValue;
import com.epam.indigolucene.common.types.values.ReactValue;
import com.epam.indigolucene.common.types.values.StringValue;
import com.epam.indigolucene.common.utils.Utils;
import indexer.data.generated.TestSchema;

public class TestSchemaDocument
    implements DocumentRepresentation<TestSchema>
{

    private StringValue<TestSchema> molId;
    private StringValue<TestSchema> docId;
    private StringValue<TestSchema> testMultiVal;
    private StringValue<TestSchema> docUrl;
    private ReactValue<TestSchema> react;
    private MolValue<TestSchema> mol;
    private StringValue<TestSchema> contentType;
    private StringValue<TestSchema> reactId;

    public TestSchemaDocument() {
    }

    @Override
    public Map<String, Object> fieldsMap() {
        Map<String, Object> res = new HashMap();
        Utils.addValueToFieldsMap(molId, res);
        Utils.addValueToFieldsMap(docId, res);
        Utils.addValueToFieldsMap(testMultiVal, res);
        Utils.addValueToFieldsMap(docUrl, res);
        Utils.addValueToFieldsMap(react, res);
        Utils.addValueToFieldsMap(mol, res);
        Utils.addValueToFieldsMap(contentType, res);
        Utils.addValueToFieldsMap(reactId, res);
        return res;
    }

    public TestSchemaDocument setMolId(String val) {
        this.molId = TestSchema.MOL_ID.createValue(val);
        return this;
    }

    public TestSchemaDocument setDocId(String val) {
        this.docId = TestSchema.DOC_ID.createValue(val);
        return this;
    }

    public TestSchemaDocument setTestMultiVal(String val) {
        this.testMultiVal = TestSchema.TEST_MULTI_VAL.createValue(val);
        return this;
    }

    public TestSchemaDocument setDocUrl(String val) {
        this.docUrl = TestSchema.DOC_URL.createValue(val);
        return this;
    }

    public TestSchemaDocument setReact(IndigoObject val) {
        this.react = TestSchema.REACT.createValue(val);
        return this;
    }

    public TestSchemaDocument setMol(IndigoObject val) {
        this.mol = TestSchema.MOL.createValue(val);
        return this;
    }

    public TestSchemaDocument setContentType(String val) {
        this.contentType = TestSchema.CONTENT_TYPE.createValue(val);
        return this;
    }

    public TestSchemaDocument setReactId(String val) {
        this.reactId = TestSchema.REACT_ID.createValue(val);
        return this;
    }

}
