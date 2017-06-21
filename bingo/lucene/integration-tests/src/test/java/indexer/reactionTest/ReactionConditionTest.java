package indexer.reactionTest;

import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.query.BeforeGroup;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.log4j.Logger;
import org.junit.Assert;
import org.junit.Test;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import static indexer.data.generated.TestSchema.*;

/**
 * The ReactionConditionTest class contains integration tests for all currently available molecular structure
 * search methods.
 *
 * @author Filipp Pisarev
 * @since 2017-03-30
 */
public class ReactionConditionTest extends ReactionBaseTest {
    private static final Logger logger = Logger.getLogger(ReactionConditionTest.class);

    private static final int REACITON_SMALL_LIMIT = 2;
    private static final int REACTION_BIG_LIMIT = 2000;
    private static final int RARE_REACTION_LIMIT = 2000;

    private static final String CORE_NAME = "moldocs";
    private static final String RARE_REACTION = "[I-].[Na+].C=CCBr>>[Na+].[Br-].C=CCI";

    @Test
    public void reactionBenchmarkTest() throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Warming up query");
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(REACT.unsafeHasSubstructure(RARE_REACTION)).limit(1).processWith(lst -> logger.info(lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");

        benchmarkReaction(RARE_REACTION, REACTION_BIG_LIMIT);
        benchmarkReaction(RARE_REACTION, REACITON_SMALL_LIMIT);
        benchmarkReaction(RARE_REACTION, RARE_REACTION_LIMIT);
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
    public void testExactMatchesSearch() throws Exception {
        testCollection.removeAll();
        String[] variousTextValues = {"matchReaction", "react"};

        try(SolrUploadStream ustream = testCollection.uploadStream()) {
            for(String varTextValue:variousTextValues) {
                TestSchemaDocument emptyDocument;
                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(varTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(REACTION));
                ustream.addDocument(emptyDocument);

                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(varTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction( "CBr>>CClC"));
                ustream.addDocument(emptyDocument);
            }
        }

        logger.info("done");

        List<Map<String, Object>> result = new LinkedList<>();

        testCollection.find().filter(CONTENT_TYPE.startsWith(variousTextValues[0])).
                filter(REACT.unsafeExactMatches(REACTION)).
                processWith(lst -> result.addAll(lst));
        logger.info(result + " : " + result.size());
        System.out.println(result.size());
        Assert.assertTrue(result.size() == 1);
        result.clear();
    }


    @Test
    public void testReactionSubstructureSearch() throws Exception{
        testCollection.removeAll();
        String[] variousReactTextValues = {"react1", "react2"};

        try (SolrUploadStream ustream = testCollection.uploadStream()) {
            for (String variousTextValue : variousReactTextValues) {
                TestSchemaDocument emptyDocument;
                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(REACTION));
                ustream.addDocument(emptyDocument);

                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction("CBr>>CClC"));
                ustream.addDocument(emptyDocument);
            }
        }
        logger.info("done");

        List<Map<String, Object>> result = new LinkedList<>();

        testCollection.find().
                filter(REACT.unsafeHasSubstructure(REACTION)).
                processWith(lst -> result.addAll(lst));
        logger.info(result + " : " + result.size());
        System.out.println(result.size());
        Assert.assertTrue(result.size() == 4);
        result.clear();
    }

    @Test
    public void testSimilarSearch() throws Exception{
        List<Map<String, Object>> result = new LinkedList<>();
        BeforeGroup<TestSchema> query = testCollection.find().filter(REACT.unsafeIsSimilarTo(RARE_REACTION)).limit(2);
        query.processWith(result::addAll);
        logger.info("Search is complete");
        result.stream().map((e) -> "Chemical element is found: " + e).forEach(System.out::println);
    }

    protected void benchmarkReaction(String reaction, int limit) throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Searching " + reaction + " with limit " + limit);
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(REACT.unsafeHasSubstructure(reaction)).limit(limit).processWith(lst -> logger.info("returned results: " + lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");
    }
}
