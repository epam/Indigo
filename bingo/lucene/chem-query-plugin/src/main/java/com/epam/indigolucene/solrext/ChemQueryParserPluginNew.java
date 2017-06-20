package com.epam.indigolucene.solrext;

import org.apache.commons.codec.binary.Base64;
import org.apache.log4j.Logger;
import org.apache.lucene.search.Query;
import org.apache.solr.common.params.ModifiableSolrParams;
import org.apache.solr.common.params.SolrParams;
import org.apache.solr.common.util.NamedList;
import org.apache.solr.request.SolrQueryRequest;
import org.apache.solr.search.LuceneQParser;
import org.apache.solr.search.QParser;
import org.apache.solr.search.QParserPlugin;
import org.apache.solr.search.SyntaxError;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
/**
 * Custom query parser implementation.
 *
 * @author Artem Malykh
 * created on 2016-02-16
 */
public class ChemQueryParserPluginNew extends QParserPlugin  {
    private static final Logger logger = Logger.getLogger(ChemQueryParserPluginNew.class);

    @Override
    public QParser createParser(String qstr, SolrParams localParams,
                                SolrParams params, SolrQueryRequest req) {

        //Modify params. Make fingerprint query from qmol, make request by fingerprint and then filter candidates by actual mol.

        try {
            String qStr = params.get(com.epam.indigolucene.common.query.Query.JSON_QUERY_PARAM);
            JSONObject jsonQuery = (JSONObject) new JSONParser().parse(new String(Base64.decodeBase64(qStr)));

            com.epam.indigolucene.common.query.Query originalQuery = com.epam.indigolucene.common.query.Query.fromJson((JSONObject) jsonQuery);
            ModifiableSolrParams modParams = new ModifiableSolrParams(params);

            ModifiableSolrParams modLocalParams = new ModifiableSolrParams(localParams);

            return new LuceneQParser("*:*", modLocalParams, modParams, req) {
                @Override
                public Query parse() throws SyntaxError {
                    Query superQuery = super.parse();
                    return new ChemFilter(superQuery, originalQuery);
                }
            };
        } catch (ParseException e) {
            e.printStackTrace();
            logger.error("error parsing query");
            return null;
        }
    }

    @Override
    public void init(NamedList args) {
        logger.info("init of chem query parser plugin");
    }
}
