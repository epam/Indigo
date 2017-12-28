package indexer.moleculeTest;

import com.epam.indigolucene.common.exceptions.RemoveException;
import com.epam.indigolucene.common.query.BeforeGroup;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import org.apache.log4j.Logger;
import org.junit.*;
import org.junit.runners.MethodSorters;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import static indexer.data.generated.TestSchema.MOL;

/**
 * Source:      SolrMoleculeConditionLoadTest.java
 * Created:     09.10.2017
 * Project:     parent-project
 * <p>
 * {@code SolrMoleculeConditionLoadTest} Base tests with some useful methods for extended testing.
 *
 * @author Dmitrii Kuznetsov
 */
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class SolrMoleculeConditionLoadTest extends MoleculeBaseLoadTest {
    private static final Logger logger = Logger.getLogger(SolrMoleculeConditionLoadTest.class);

    private static final String MOL_FILE_1MILLION = "pubchem_slice_1m.smiles";
    private static final String MOL_FILE_100K = "pubchem_slice_100k.smiles";
    private static final String MOL_FILE_10K = "pubchem_slice_10k.smiles";

    private static final String MOL_FILE_NAME = MOL_FILE_1MILLION;

    static final String TEST_CORE_NAME = "moldocs";

    private static final String SEARCH_MOL = "O1C2=C(C=C(OCC=C)C(CC3=CC(OC)=C(OC)C(OC)=C3)=C2)OC1";

    @BeforeClass
    public static void beforeClass() throws Exception {
        SolrConnectionFactory.clear();
        SolrConnectionFactory.init(SolrConnection5.class);
        testCollection = TestSchema.collection(ServiceConfig.SERVICE_URL, TEST_CORE_NAME);
        testCollection.removeAll();
    }

    @AfterClass
    public static void afterClass() throws RemoveException {
        testCollection.removeAll();
    }

    @Test
    public void test01LoadMillionMolecules() throws Exception {
        checkTime(this::loadMillionMoleculesFunction);
    }

    @Test
    public void test02SimilarSearch() {
        checkTime(this::similarSearch);
    }

    private void loadMillionMoleculesFunction() {
        try {
            indexSmilesFile(MOL_FILE_NAME);
        } catch (Exception e) {
            logException(e);
        }
    }

    private void similarSearch() {
        List<Map<String, Object>> result = new LinkedList<>();
        try {
            BeforeGroup<TestSchema> query = testCollection.find().filter(MOL.unsafeIsSimilarTo(SEARCH_MOL));
            query.processWith(result::addAll);
        } catch (Exception e) {
            logException(e);
        }
        System.out.println("\nFound " + result.size() + " elements\n");
        getLogger().debug("Found " + result.size() + " elements");

        Assert.assertTrue(result.size() != 0);
    }

    private void checkTime(Runnable testFunction) {
        long startTime = System.nanoTime();
        testFunction.run();
        long endTime = System.nanoTime();
        long duration = (endTime - startTime);

        System.out.println("\nDURATION: " + TimeUnit.NANOSECONDS.toMillis(duration) + "milliseconds");
        System.out.println("DURATION: " + TimeUnit.NANOSECONDS.toSeconds(duration) + "seconds");
        System.out.println("DURATION: " + TimeUnit.NANOSECONDS.toMinutes(duration) + "minutes\n");
    }

    Logger getLogger() {
        return SolrMoleculeConditionLoadTest.logger;
    }

    private void logException(Exception e) {
        getLogger().error(e.getMessage());
        getLogger().error(e);
        e.printStackTrace();
    }
}
