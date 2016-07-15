package com.epam.indigolucene.solrext;

import org.apache.lucene.search.similarities.ClassicSimilarity;
import org.apache.lucene.search.similarities.DefaultSimilarity;

/**
 * This class encapsulated chem similarity.
 * Created by Artem Malykh on 19.04.16.
 */
public class ChemSimilarity extends ClassicSimilarity {

    /**
     * We want to have invariant
     * filter (similarity > s) docCollection subset filter (similarity > s) (docCollection union otherDocCollection).
     * In other words we want not to lose documents from result set after adding other documents in collection.
     * Having classical idf can break this invariant. Suppose we add many documents which contain given term. Then its
     * idf will decrease, so its score and we will lose some documents from new result set.
     *
     * Actually, we should carefully think about this need. The price for having this invariant is losing ability to boost
     * more distinguishing features from less distinguishing features.
     *
     * @param docFreq number of documents having given term
     * @param numDocs total number of documents
     * @return inverse document frequency
     */
    @Override
    public float idf(long docFreq, long numDocs) {
        return 1.0f;
    }
}
