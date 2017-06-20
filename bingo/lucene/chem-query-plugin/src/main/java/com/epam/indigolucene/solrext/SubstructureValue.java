package com.epam.indigolucene.solrext;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.ValueSource;

import java.io.IOException;
import java.util.Map;
/**
 * This class represents a methods for substructure value determination.
 *
 * @author Artem Malykh
 * created on 2016-03-01
 */
public class SubstructureValue extends ValueSource {
    private final IndigoObject qChem;
    private String fieldName;

    public SubstructureValue(IndigoObject qChem, String fieldName) {
        this.qChem = qChem;
        this.fieldName = fieldName;
    }

    @Override
    public FunctionValues getValues(Map map, LeafReaderContext leafReaderContext) throws IOException {
        return new FunctionValues() {
            @Override
            public float floatVal(int doc) {
                LeafReader reader = leafReaderContext.reader();
                Document curDoc = null;
                try {
                    curDoc = reader.document(doc);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                IndigoObject chem = getChem(fieldName, curDoc);
                if (IndigoHolder.getIndigo().substructureMatcher(chem).match(qChem) != null) {
                    return 1.0f;
                } else {
                    return 0.0f;
                }
            }

            @Override
            public int intVal(int doc) {
                return super.intVal(doc);
            }

            @Override
            public String toString(int i) {
                return null;
            }


        };
    }

    private IndigoObject getChem(String fieldName, Document doc) {
        return IndigoHolder.getIndigo().unserialize(doc.getBinaryValue(fieldName).bytes);
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        SubstructureValue that = (SubstructureValue) o;

        if (qChem != null ? !qChem.equals(that.qChem) : that.qChem != null) return false;
        return fieldName != null ? fieldName.equals(that.fieldName) : that.fieldName == null;

    }

    @Override
    public int hashCode() {
        int result = qChem != null ? qChem.hashCode() : 0;
        result = 31 * result + (fieldName != null ? fieldName.hashCode() : 0);
        return result;
    }

    @Override
    public String description() {
        return null;
    }
}
