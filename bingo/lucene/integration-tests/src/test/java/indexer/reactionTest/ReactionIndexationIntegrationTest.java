package indexer.reactionTest;

import com.epam.indigo.Indigo;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;

/**
 * Created by Filipp Pisarev on 05/04/2017.
 *
 */
public class ReactionIndexationIntegrationTest extends ReactionBaseTest {

    private static Indigo indigo = new Indigo();

    @BeforeClass
    public static void init() throws IOException, SolrServerException {
        SolrConnectionFactory.init(SolrConnection5.class);
    }

    @Test
    public void addDoc() throws Exception {
        TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();

        testCollection.removeAll();
        try (SolrUploadStream<TestSchema> uStream = testCollection.uploadStream()) {
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
