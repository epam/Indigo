package com.epam.indigolucene.solrext;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.ChemStructureCondition;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.QueryWrapperFilter;
import org.apache.solr.search.DelegatingCollector;
import org.apache.solr.search.ExtendedQuery;
import org.apache.solr.search.PostFilter;
import org.apache.solr.search.SolrConstantScoreQuery;

import java.io.IOException;
import java.util.List;
/**
 * This class encapsulates main logic of server-side additional query parameters parsing, such as offset, limit,  etc.
 *
 * @author Artem Malykh
 * created on 2016-03-31
 */
public class ChemFilter extends SolrConstantScoreQuery implements PostFilter {

    private List<ChemStructureCondition> chemConditions;
    private int cost;
    private long offset;
    private long limit;

    private boolean cacheSep;

    public ChemFilter(Query filter, com.epam.indigolucene.common.query.Query originalQuery) {
        super(new QueryWrapperFilter(filter));
        chemConditions = originalQuery.getCondition().chemStructureConditions();
        offset = originalQuery.getOffset();
        //TODO: workaraund against solr 'start' parameter cuts results after filtering. Very inefficient since it enlarges number of documents to filter.
        limit = originalQuery.getLimit() + offset;
    }

    @Override
    public DelegatingCollector getFilterCollector(IndexSearcher indexSearcher) {
        return new DelegatingCollector() {
            private long totalDocsFound = 0;

            private boolean checkLimit() {
                return (limit <= 0 || (totalDocsFound <= limit));
            }

            @Override
            public void collect(int doc) throws IOException {

                if (checkLimit()) {
                    LeafReader reader = context.reader();
                    Document curDoc = reader.document(doc);
                    if (checkChem(curDoc)) {
                        totalDocsFound++;
                        super.collect(doc);
                    }
                }
            }
        };
    }

    private boolean checkChem(Document curDoc) {
        Indigo indigo = IndigoHolder.getIndigo();
        for (ChemStructureCondition chemCondition : chemConditions) {
            byte[] serializedChem = curDoc.getField(chemCondition.getFieldName()).binaryValue().bytes;
            IndigoObject chem = indigo.unserialize(serializedChem);
            if (!chemCondition.match(chem)) {
                return false;
            }
        }
        return true;
    }

    public boolean getCache() {
        return false;
    }

    @Override
    public void setCache(boolean b) {}

    public void setCacheSep(boolean cacheSep) {
        this.cacheSep = cacheSep;
    }

    public boolean getCacheSep() {
        return this.cacheSep;
    }

    public void setCost(int cost) {
        this.cost = cost;
    }

    public int getCost() {
        return Math.max(this.cost, 1000);
    }

    private String getOptions() {
        return getOptionsString(this);
    }

    private static String getOptionsString(ExtendedQuery q) {
        StringBuilder sb = new StringBuilder();
        if(!q.getCache()) {
            sb.append("{!cache=false");
            int cost = q.getCost();
            if(cost != 0) {
                sb.append(" cost=");
                sb.append(q.getCost());
            }

            sb.append("}");
        } else if(q.getCacheSep()) {
            sb.append("{!cache=sep");
            sb.append("}");
        }

        return sb.toString();
    }



    public String toString(String field) {
        return this.getOptions();
    }
}
