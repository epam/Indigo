
package com.epam.indigolucene.service.generated;

import java.util.HashMap;
import java.util.Map;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.DocumentRepresentation;
import com.epam.indigolucene.common.types.values.MolValue;
import com.epam.indigolucene.common.types.values.StringValue;
import com.epam.indigolucene.common.utils.Utils;
import com.epam.indigolucene.service.generated.TestSchema;

public class TestSchemaDocument
    implements DocumentRepresentation<TestSchema>
{

    private StringValue<TestSchema> molId;
    private MolValue<TestSchema> mol;

    public TestSchemaDocument() {
    }

    @Override
    public Map<String, Object> fieldsMap() {
        Map<String, Object> res = new HashMap();
        Utils.addValueToFieldsMap(molId, res);
        Utils.addValueToFieldsMap(mol, res);
        return res;
    }

    public TestSchemaDocument setMolId(String val) {
        this.molId = TestSchema.MOL_ID.createValue(val);
        return this;
    }

    public TestSchemaDocument setMol(IndigoObject val) {
        this.mol = TestSchema.MOL.createValue(val);
        return this;
    }

}
