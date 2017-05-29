
package com.epam.indigolucene.service.generated;

import java.util.HashMap;
import java.util.Map;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.DocumentRepresentation;
import com.epam.indigolucene.common.types.values.ChemValue;
import com.epam.indigolucene.common.types.values.StringValue;
import com.epam.indigolucene.common.utils.Utils;
import com.epam.indigolucene.service.generated.TestSchema;

public class TestSchemaDocument
    implements DocumentRepresentation<TestSchema>
{

    private StringValue<TestSchema> chemId;
    private ChemValue<TestSchema> chem;
    private StringValue<TestSchema> docId;
    private StringValue<TestSchema> testMultiVal;
    private StringValue<TestSchema> docUrl;
    private StringValue<TestSchema> contentType;

    public TestSchemaDocument() {
    }

    @Override
    public Map<String, Object> fieldsMap() {
        Map<String, Object> res = new HashMap();
        Utils.addValueToFieldsMap(chemId, res);
        Utils.addValueToFieldsMap(chem, res);
        Utils.addValueToFieldsMap(docId, res);
        Utils.addValueToFieldsMap(testMultiVal, res);
        Utils.addValueToFieldsMap(docUrl, res);
        Utils.addValueToFieldsMap(contentType, res);
        return res;
    }

    public TestSchemaDocument setChemId(String val) {
        this.chemId = TestSchema.CHEM_ID.createValue(val);
        return this;
    }

    public TestSchemaDocument setChem(IndigoObject val) {
        this.chem = TestSchema.CHEM.createValue(val);
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

    public TestSchemaDocument setContentType(String val) {
        this.contentType = TestSchema.CONTENT_TYPE.createValue(val);
        return this;
    }

}
