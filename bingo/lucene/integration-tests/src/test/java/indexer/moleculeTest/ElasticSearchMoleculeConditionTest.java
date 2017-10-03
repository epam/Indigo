package indexer.moleculeTest;

import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import com.epam.indigolucene.solrconnection.elastic.ElasticConnection;
import indexer.data.generated.TestSchema;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.*;

import java.io.IOException;


/**
 * Note! These are not fully functional tests yet. This suit was rather experimental board
 * for quick checks working of plugin. This class should be redone.
 */
//TODO: move to core package
public class ElasticSearchMoleculeConditionTest extends MoleculeConditionTest {

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
    public void test1() throws Exception {
        super.test1();
    }

    @Test
    @Override
    public void testSimilarSearch() throws Exception {
        super.testSimilarSearch();
    }

    @Test
    @Override
    public void testMoleculeTextSearch() throws Exception {
        super.testMoleculeTextSearch();
    }
}
