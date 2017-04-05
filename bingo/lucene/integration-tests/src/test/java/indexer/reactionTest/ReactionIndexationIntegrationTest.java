package indexer.reactionTest;

import com.epam.indigo.Indigo;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.log4j.Logger;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;

/**
 * Created by Filipp Pisarev on 05/04/2017.
 */
public class ReactionIndexationIntegrationTest extends ReactionBaseTest {
    private long MAX_AVERAGE = 15;

    static Logger logger = Logger.getLogger(ReactionIndexationIntegrationTest.class);
    private static Indigo indigo;

    @BeforeClass
    public static void init() throws IOException, SolrServerException {
        indigo = new Indigo();
        SolrConnectionFactory.init(SolrConnection5.class);
    }


    @Test
    public void addDoc() throws Exception {
        TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();

        testCollection.removeAll();
        try (SolrUploadStream<TestSchema> uStream = TestSchema.collection(ServiceConfig.SERVICE_URL, "").uploadStream()) {
            emptyDocument.setMol(indigo.loadReaction(REACTION));
            uStream.addDocument(emptyDocument);
        }
    }

    @Test
    public void indexChemul() throws Exception {
        indexSDFile(ReactionIndexationIntegrationTest.class.getClassLoader().getResource("all_chemul.sd").getFile());
    }

    //@Test
    public void indexPubchem10M() throws Exception {
        indexSmilesFile(ReactionIndexationIntegrationTest.class.getClassLoader().getResource("pubchem_slice_10000000.smiles").getFile());
    }
}
