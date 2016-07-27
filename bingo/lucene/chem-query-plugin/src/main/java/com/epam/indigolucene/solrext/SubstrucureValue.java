package com.epam.indigolucene.solrext;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.DocValues;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.queries.function.FunctionValues;
import org.apache.lucene.queries.function.ValueSource;

import java.io.IOException;
import java.util.Map;

/**
 * Created by Artem Malykh on 01.03.16.
 */
public class SubstrucureValue extends ValueSource {
    private final IndigoObject qMol;
    private String fieldName;

    public SubstrucureValue(IndigoObject qMol, String fieldName) {
        this.qMol = qMol;
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
                IndigoObject mol = getMol(fieldName, curDoc);
                if (IndigoHolder.getIndigo().substructureMatcher(mol).match(qMol) != null) {
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

    private IndigoObject getMol(String fieldName, Document doc) {
        return IndigoHolder.getIndigo().unserialize(doc.getBinaryValue(fieldName).bytes);
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        SubstrucureValue that = (SubstrucureValue) o;

        if (qMol != null ? !qMol.equals(that.qMol) : that.qMol != null) return false;
        return fieldName != null ? fieldName.equals(that.fieldName) : that.fieldName == null;

    }

    @Override
    public int hashCode() {
        int result = qMol != null ? qMol.hashCode() : 0;
        result = 31 * result + (fieldName != null ? fieldName.hashCode() : 0);
        return result;
    }

    @Override
    public String description() {
        return null;
    }
}
