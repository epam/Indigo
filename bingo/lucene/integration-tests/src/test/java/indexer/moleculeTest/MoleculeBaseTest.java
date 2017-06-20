package indexer.moleculeTest;

import com.epam.indigo.IndigoException;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.solr.client.solrj.SolrServerException;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

/**
 * Base tests with some useful methods for extended testing.
 * Created by Artem_Malykh on 9/9/2015.
 */
public class MoleculeBaseTest {
    private static final Logger logger = Logger.getLogger(MoleculeBaseTest.class);

    public static final String BENZOL         = "c1ccccc1";
    public static final String TEST_CORE_NAME = "moldocs";

    CollectionRepresentation<TestSchema> testCollection;

    public static class IndexingStatistics {
        public final long totalTime;
        public final long averageTime;
        public final long totalItems;

        public IndexingStatistics(long totalTime, long averageTime, long totalItems) {
            this.totalTime = totalTime;
            this.averageTime = averageTime;
            this.totalItems = totalItems;
        }
    }

    static {
        BasicConfigurator.configure();
        Logger.getRootLogger().setLevel(Level.INFO);
    }

    @BeforeClass
    public static void beforeClass() {
        SolrConnectionFactory.init(SolrConnection5.class);
    }



    @Before
    public void before() throws IOException, SolrServerException {
        testCollection = TestSchema.collection(ServiceConfig.SERVICE_URL, TEST_CORE_NAME);
    }

    protected void indexSDFile(String fileName) throws Exception {
        try (SolrUploadStream<TestSchema> uploadStream = testCollection.uploadStream()) {
            for (IndigoObject mol : IndigoHolder.getIndigo().iterateSDFile(fileName)) {
                try {
                    TestSchemaDocument doc = TestSchema.createEmptyDocument();
                    doc.setMol(mol);
                    uploadStream.addDocument(doc);
                    System.out.println(mol.smiles());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    protected void indexSmilesFile(String fileName) throws Exception {
        try (SolrUploadStream<TestSchema> uploadStream = testCollection.uploadStream()) {
            BufferedReader br = new BufferedReader(new FileReader(fileName));
            String smilesLine;
            while ((smilesLine = br.readLine()) != null) {
                try {
                    IndigoObject indigoObject = IndigoHolder.getIndigo().loadMolecule(smilesLine);
                    TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();

                    emptyDocument.setMol(indigoObject);

                    uploadStream.addDocument(emptyDocument);
                } catch (IndigoException e) {

                }
            }
        }
    }

    @After
    public void cleanup() throws IOException, SolrServerException, RemoveException {
        logger.info("cleanup...");
//        testCollection.removeAll();
    }

    /**
     * Force deletion of directory
     * @param dir Directory to delete
     * @return Success if directory was deleted.
     */
    static boolean deleteDirectory(File dir) {
        boolean res = true;
        if (dir.exists()) {
            File[] files = dir.listFiles();
            if (files != null) {
                for (int i = 0; i < files.length; i++) {
                    if (files[i].isDirectory()) {
                        res &= deleteDirectory(files[i]);
                    } else {
                        res &= files[i].delete();
                    }
                }
            }
        }
        return res & dir.delete();
    }
}
