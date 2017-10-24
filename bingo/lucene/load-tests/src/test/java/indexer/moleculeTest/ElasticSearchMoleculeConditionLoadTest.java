package indexer.moleculeTest;

import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.elastic.ElasticConnection;
import indexer.data.generated.TestSchema;
import org.apache.log4j.Logger;
import org.junit.*;
import org.junit.runners.MethodSorters;

/**
 * Source:      ElasticSearchMoleculeConditionLoadTest.java
 * Created:     09.10.2017
 * Project:     parent-project
 * <p>
 * {@code ElasticSearchMoleculeConditionLoadTest} Base tests with some useful methods for extended testing.
 *
 * @author Dmitrii Kuznetsov
 */
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class ElasticSearchMoleculeConditionLoadTest extends SolrMoleculeConditionLoadTest {
    private static final Logger logger = Logger.getLogger(ElasticSearchMoleculeConditionLoadTest.class);

    @BeforeClass
    public static void beforeClass() throws Exception {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(ElasticConnection.class);
        testCollection = TestSchema.collection(ServiceConfig.ELASTIC_URL, TEST_CORE_NAME);
        testCollection.removeAll();
    }

    @AfterClass
    public static void afterClass() throws RemoveException {
        testCollection.removeAll();
    }

    @Test
    @Override
    public void test01LoadMillionMolecules() throws Exception {
        super.test01LoadMillionMolecules();
    }

    @Test
    @Override
    public void test02SimilarSearch() {
        super.test02SimilarSearch();
    }

    @Override
    Logger getLogger() {
        return ElasticSearchMoleculeConditionLoadTest.logger;
    }


}
