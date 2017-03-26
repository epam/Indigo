package com.epam.indigolucene.solrext;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.types.conditions.molconditions.MolStructureCondition;
import com.epam.indigolucene.common.types.conditions.reactconditions.ReactStructureCondition;
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
 * Created by Artem Malykh on 31.03.16.
 */
public class ChemFilter extends SolrConstantScoreQuery implements PostFilter {

    private List<MolStructureCondition> molConditions;
    private List<ReactStructureCondition> reactConditions;
    private int cost;
    private long offset;
    private long limit;

    private boolean cacheSep;

    public ChemFilter(Query filter, com.epam.indigolucene.common.query.Query originalQuery) {
        super(new QueryWrapperFilter(filter));
        molConditions = originalQuery.getCondition().molStructureConditions();
        reactConditions = originalQuery.getCondition().reactStructureConditions();
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

        for (MolStructureCondition molCondition : molConditions) {
            byte[] serializedMol = curDoc.getField(molCondition.getFieldName()).binaryValue().bytes;
            IndigoObject mol = indigo.unserialize(serializedMol);
            //If no molecule found, keep digging for reactions
            if (!molCondition.match(mol)) {
                for (ReactStructureCondition reactCondition : reactConditions) {
                    byte[] serializedReact = curDoc.getField(reactCondition.getFieldName()).binaryValue().bytes;
                    IndigoObject react = indigo.unserialize(serializedReact);
                    if (!reactCondition.match(react)) {
                        return false;
                    }
                }
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
