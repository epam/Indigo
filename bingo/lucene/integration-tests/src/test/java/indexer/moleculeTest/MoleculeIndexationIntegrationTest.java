package indexer.moleculeTest;

import com.epam.indigo.Indigo;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import indexer.reactionTest.ReactionIndexationIntegrationTest;
import org.apache.log4j.Logger;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.URL;

/**
 * Indexation tests.performance test
 * Created by Artem_Malykh on 9/8/2015.
 */
public class MoleculeIndexationIntegrationTest extends MoleculeBaseTest {

    private static final Logger logger = Logger.getLogger(MoleculeIndexationIntegrationTest.class);
    private static Indigo indigo = new Indigo();

    @BeforeClass
    public static void beforeClass() {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(SolrConnection5.class);
    }

    @Test
    public void addDoc() throws Exception {
        TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();

        testCollection.removeAll();
        try (SolrUploadStream<TestSchema> uStream = testCollection.uploadStream()) {
            emptyDocument.setMol(indigo.loadMolecule(BENZOL));
            uStream.addDocument(emptyDocument);
        }
    }

    @Test
    public void indexChemul() throws Exception {
        URL url = ReactionIndexationIntegrationTest.class.getClassLoader().getResource("all_chemul.sd");
        if (url != null) {
            indexSDFile(url.getFile());
        } else {
            logger.error("indexChemul() test: file all_chemul.sd isn't found");
            System.err.println("indexChemul() test: file all_chemul.sd isn't found");
        }
    }

    //@Test
    public void indexPubchem10M() throws Exception {
        URL url = ReactionIndexationIntegrationTest.class.getClassLoader().getResource("pubchem_slice_10000000.smiles");
        if (url != null) {
            indexSmilesFile(url.getFile());
        } else {
            logger.error("indexPubchem10M() test: file pubchem_slice_10000000.smiles isn't found");
            System.err.println("indexPubchem10M() test: file pubchem_slice_10000000.smiles isn't found");
        }
    }
}
