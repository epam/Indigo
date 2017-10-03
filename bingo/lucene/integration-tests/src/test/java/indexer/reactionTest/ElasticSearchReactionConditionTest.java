package indexer.reactionTest;

import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import com.epam.indigolucene.solrconnection.elastic.ElasticConnection;
import indexer.data.generated.TestSchema;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;

public class ElasticSearchReactionConditionTest extends ReactionConditionTest {

    @BeforeClass
    public static void beforeClass() {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(ElasticConnection.class);
    }

    @AfterClass
    public static void destroy() {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(SolrConnection5.class);
    }

    @Before
    @Override
    public void before() throws IOException, SolrServerException {
        testCollection = TestSchema.collection(ServiceConfig.ELASTIC_URL, TEST_CORE_NAME);
    }

    @Test
    @Override
    public void reactionBenchmarkTest() throws Exception {
        super.reactionBenchmarkTest();
    }

    @Test
    @Override
    public void testExactMatchesSearch() throws Exception {
        super.testExactMatchesSearch();
    }

    @Test
    @Override
    public void testReactionSubstructureSearch() throws Exception {
        super.testReactionSubstructureSearch();
    }

    @Test
    @Override
    public void testSimilarSearch() throws Exception {
        super.testSimilarSearch();
    }
}
