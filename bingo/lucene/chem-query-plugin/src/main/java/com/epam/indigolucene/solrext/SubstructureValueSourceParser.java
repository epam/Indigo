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
 * Parsing of chemical values and transforming them to Substructure value form.
 *
 * @author Artem Malykh
 * created on 2016-03-01
 */
public class SubstructureValueSourceParser extends ValueSourceParser {
    @Override
    public void init(NamedList args) {
        super.init(args);
    }

    @Override
    public ValueSource parse(FunctionQParser fp) throws SyntaxError {
        byte[] qChemBytes = Base64.decode(fp.parseArg());
        IndigoObject qReact = IndigoHolder.getIndigo().loadQueryReaction(qChemBytes);
        String fieldName = fp.parseArg();
        return new SubstructureValue(qReact, fieldName);
    }
}
