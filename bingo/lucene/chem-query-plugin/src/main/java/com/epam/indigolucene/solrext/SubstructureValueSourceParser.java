package com.epam.indigolucene.solrext;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import org.apache.lucene.queries.function.ValueSource;
import org.apache.solr.common.util.NamedList;
import org.apache.solr.search.FunctionQParser;
import org.apache.solr.search.SyntaxError;
import org.apache.solr.search.ValueSourceParser;
import org.restlet.engine.util.Base64;

/**
 * Created by Artem Malykh on 01.03.16.
 */
public class SubstructureValueSourceParser extends ValueSourceParser {
    @Override
    public void init(NamedList args) {
        super.init(args);
    }

    @Override
    public ValueSource parse(FunctionQParser fp) throws SyntaxError {
        byte[] qMolBytes = Base64.decode(fp.parseArg());
        IndigoObject qMol = IndigoHolder.getIndigo().loadQueryMolecule(qMolBytes);
        String fieldName = fp.parseArg();
        return new SubstrucureValue(qMol, fieldName);
    }
}
