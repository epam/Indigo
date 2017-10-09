package indexer.moleculeTest;

import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.elastic.ElasticConnection;
import indexer.data.generated.TestSchema;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.*;

import java.io.IOException;

public class ElasticSearchMoleculeConditionTest extends MoleculeConditionTest {

    @BeforeClass
    public static void beforeClass() {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(ElasticConnection.class);
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
