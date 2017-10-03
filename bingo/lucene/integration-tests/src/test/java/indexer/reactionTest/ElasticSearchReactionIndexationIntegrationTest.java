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

public class ElasticSearchReactionIndexationIntegrationTest extends ReactionIndexationIntegrationTest {

    @BeforeClass
    public static void init() throws IOException, SolrServerException {
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
    public void addDoc() throws Exception {
        super.addDoc();
    }

    @Test
    @Override
    public void indexChemul() throws Exception {
        super.indexChemul();
    }

    //@Test
    @Override
    public void indexPubchem10M() throws Exception {
        super.indexPubchem10M();
    }
}
