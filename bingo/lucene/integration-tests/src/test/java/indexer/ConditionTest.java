package indexer;

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

import static indexer.data.generated.TestSchema.CONTENT_TYPE;
import static indexer.data.generated.TestSchema.MOL;
import static indexer.data.generated.TestSchema.REACT;

/**
 * Created by Artem Malykh on 24.02.16.
 *
 * Note! These are not fully functional tests yet. This suit was rather experimental board
 * for quick checks working of plugin. This class should be redone.
 */
//TODO: move to core package
public class ConditionTest extends BaseTest {
    private static final Logger logger = Logger.getLogger(ConditionTest.class);

    private static final int BENZOL_SMALL_LIMIT = 2;
    private static final int BENZOL_BIG_LIMIT   = 2000;
    private static final int RARE_MOL_LIMIT     = 2000;

    private static final String CORE_NAME = "moldocs";

    //TODO: change for some real rare mol for test set.
    private static final String RARE_MOL = BENZOL;
    private static final String RARE_REACTION = REACTION;
    @Test
    public void test1() throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Warming up query");
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(MOL.unsafeHasSubstructure(RARE_MOL)).limit(1).processWith(lst -> logger.info(lst.size()));
        //TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(REACT.unsafeHasSubstructure(RARE_REACTION)).limit(1).processWith(lst -> logger.info(lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");

        benchmarkMol(BENZOL,   BENZOL_BIG_LIMIT);
        benchmarkMol(BENZOL,   BENZOL_SMALL_LIMIT);
        benchmarkMol(RARE_MOL, RARE_MOL_LIMIT);
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
    public void testSimilarSearch() throws Exception {
        BeforeGroup<TestSchema> query = testCollection.find().filter(MOL.unsafeIsSimilarTo(RARE_MOL)).limit(2);
        query.processWith(lst -> logger.info(lst.size()));
    }

    @Test
    public void testTextSearch() throws Exception {
        testCollection.removeAll();

        String[] variousTextValues = {"val1", "val2", "react"};
        //logger.info("adding documents from set " + "" + " with string values...");
        try (SolrUploadStream ustream = testCollection.uploadStream()) {
            for (String variousTextValue : variousTextValues) {
                TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setMol(IndigoHolder.getIndigo().loadMolecule(BENZOL));
                ustream.addDocument(emptyDocument);

                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setMol(IndigoHolder.getIndigo().loadMolecule(RARE_MOL));
                ustream.addDocument(emptyDocument);

                emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType(variousTextValue);
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(RARE_REACTION));
                ustream.addDocument(emptyDocument);
            }
        }
        logger.info("done");


        List<Map<String, Object>> result = new LinkedList<>();
        testCollection.find().filter(CONTENT_TYPE.startsWith(variousTextValues[0])).
                              filter(MOL.unsafeHasSubstructure(RARE_MOL)).
                              processWith(lst -> result.addAll(lst));
        System.out.println("Result " + result);
        logger.info(result + " : " + result.size());
        Assert.assertTrue(result.size() == 2);
        result.clear();

        testCollection.find().filter(CONTENT_TYPE.startsWith(variousTextValues[2])).
                filter(REACT.unsafeHasSubstructure(RARE_REACTION)).
                processWith(lst -> result.addAll(lst));
        logger.info(result + " : " + result.size());
        Assert.assertTrue(result.size() == 1);
        result.clear();

        testCollection.find().filter(CONTENT_TYPE.startsWith(variousTextValues[0])).
                              processWith(lst -> result.addAll(lst));
        logger.info(result + " : " + result.size());
        Assert.assertTrue(result.size() == 0);
        result.clear();
    }

    protected void benchmarkMol(String mol, int limit) throws Exception {
        long before = System.currentTimeMillis();
        logger.info("Searching " + mol + " with limit " + limit);
        TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME).find().filter(MOL.unsafeHasSubstructure(mol)).limit(limit).processWith(lst -> logger.info("returned results: " + lst.size()));
        logger.info("Took approx. " + (System.currentTimeMillis() - before) + " ms ");
    }
}
