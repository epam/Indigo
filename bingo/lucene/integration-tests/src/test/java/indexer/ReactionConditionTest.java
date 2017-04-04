package indexer;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.log4j.Logger;
import org.junit.Assert;
import org.junit.Test;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import static indexer.data.generated.TestSchema.CONTENT_TYPE;
import static indexer.data.generated.TestSchema.REACT;

/**
 * Created by Filipp Pisarev on 30/03/2017.
 */
public class ReactionConditionTest extends BaseTest {
    private static final Logger logger = Logger.getLogger(ReactionConditionTest.class);

    private static final int REACITON_SMALL_LIMIT = 2;
    private static final int REACTION_BIG_LIMIT = 2000;
    private static final int RARE_REACTION_LIMIT = 2000;

    private static final String CORE_NAME = "moldocs";
    private static final String RARE_REACTION = REACTION;

    @Test
    public void reactionBenchmarkTest() throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Warming up query");
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(REACT.unsafeHasSubstructure(RARE_REACTION)).limit(1).processWith(lst -> logger.info(lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");

        benchmarkMol(BENZOL, REACTION_BIG_LIMIT);
        benchmarkMol(BENZOL, REACITON_SMALL_LIMIT);
        benchmarkMol(RARE_REACTION, RARE_REACTION_LIMIT);
        /**
         * 60k   --- 1700, ?,   230, 61
         * 90k   --- 1700, ?,   299, 62
         * 350k  --- 2000, 85,  329, 110d
         * 740k  --- 2200, 176, 409, 183
         * 1000k --- 1966, 197, 421, 155
         * 1200k --- 1900, 192, 376, 115
         * 1380k --- 1900, 210, 399, 124
         * 1700k --- 5, 210, 399, 124
         * 2100k --- 1600, 229, 511, 158
         */
    }


    @Test
    public void testReactionTextSearch() throws Exception{
        testCollection.removeAll();
        String[] variousReactTextValues = {"react1", "react2"};
        String[] array1, array2;

        try (SolrUploadStream ustream = testCollection.uploadStream()) {
            for (String variousTextValue : variousReactTextValues) {
                TestSchemaDocument emptyDocument;
                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(REACTION));
                ustream.addDocument(emptyDocument);

                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(RARE_REACTION));
                ustream.addDocument(emptyDocument);
            }
        }
        logger.info("done");

        List<Map<String, Object>> result = new LinkedList<>();

        testCollection.find().filter(CONTENT_TYPE.startsWith(variousReactTextValues[0])).
                filter(REACT.unsafeHasSubstructure(RARE_REACTION)).
                processWith(lst -> result.addAll(lst));
        logger.info(result + " : " + result.size());
        Assert.assertTrue(result.size() == 1);
        result.clear();
    }

    protected void benchmarkMol(String reaction, int limit) throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Searching " + reaction + " with limit " + limit);
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(REACT.unsafeHasSubstructure(reaction)).limit(limit).processWith(lst -> logger.info("returned results: " + lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");
    }
}
