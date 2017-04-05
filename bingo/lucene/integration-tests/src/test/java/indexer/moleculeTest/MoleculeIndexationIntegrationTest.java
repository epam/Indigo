package indexer.moleculeTest;

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
 * Indexation tests.performance test
 * Created by Artem_Malykh on 9/8/2015.
 */
public class MoleculeIndexationIntegrationTest extends MoleculeBaseTest {
    private static final String COMPOSITION = "C6H6";
    private long MAX_AVERAGE = 15;

    static Logger logger = Logger.getLogger(MoleculeIndexationIntegrationTest.class);
    private static Indigo indigo;

    @BeforeClass
    public static void init() throws IOException, SolrServerException {
        indigo = new Indigo();
        SolrConnectionFactory.init(SolrConnection5.class);
    }


    @Test
    public void addDoc() throws Exception {
        TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();

        //emptyDocument.setfullMol(indigo.loadMolecule(BENZOL));

        testCollection.removeAll();
        try (SolrUploadStream<TestSchema> uStream = TestSchema.collection(ServiceConfig.SERVICE_URL, "").uploadStream()) {
            //emptyDocument.setTestMultiVal(Arrays.asList("a", "b"));
            emptyDocument.setMol(indigo.loadMolecule(BENZOL));
            uStream.addDocument(emptyDocument);
        }
    }

    @Test
    public void indexChemul() throws Exception {
        //apiImpl.removeAllDocs();
        indexSDFile(MoleculeIndexationIntegrationTest.class.getClassLoader().getResource("all_chemul.sd").getFile());
    }

    //@Test
    public void indexPubchem10M() throws Exception {
        indexSmilesFile(MoleculeIndexationIntegrationTest.class.getClassLoader().getResource("pubchem_slice_10000000.smiles").getFile());
    }
}
