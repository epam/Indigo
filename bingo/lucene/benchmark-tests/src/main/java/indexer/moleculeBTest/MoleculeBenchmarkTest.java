package indexer.moleculeBTest;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.query.SolrConnectionFactory;
import com.epam.indigolucene.commonconfig.ServiceConfig;
import com.epam.indigolucene.solrconnection.SolrConnection5;
import indexer.data.generated.TestSchema;
import indexer.data.generated.TestSchemaDocument;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.infra.Blackhole;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import static indexer.data.generated.TestSchema.MOL;

/**
 * The MoleculeBenchmarkTest class contains benchmark tests for all currently available molecular structure
 * search methods.
 *
 * @author Filipp Pisarev
 * @since 2017-04-17
 */
public class MoleculeBenchmarkTest {

    static {
        BasicConfigurator.configure();
        Logger.getRootLogger().setLevel(org.apache.log4j.Level.INFO);
    }

    @State(Scope.Thread)
    public static class SolrAndMoleculesData {
        @Setup(Level.Trial)
        public void doSetup() throws Exception {
            SolrConnectionFactory.init(SolrConnection5.class);
            testCollection = TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME);

            try (SolrUploadStream ustream = testCollection.uploadStream()) {
                TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType("molValue");
                emptyDocument.setMol(IndigoHolder.getIndigo().loadMolecule(BENZOL));
                ustream.addDocument(emptyDocument);
            }
        }
        public CollectionRepresentation<TestSchema> testCollection;
        public List<Map<String, Object>> result;
        public final  String BENZOL = "c1ccccc1";
        public final String CORE_NAME = "moldocs";
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.SECONDS)
    public void molSimilaritySearchBenchmark(SolrAndMoleculesData solrMolData, Blackhole blackhole) throws Exception {
        solrMolData.result = new LinkedList<>();
        solrMolData.testCollection.find().filter(MOL.unsafeIsSimilarTo(solrMolData.BENZOL)).
                processWith(solrMolData.result::addAll);
        blackhole.consume(solrMolData.result);
    }
}
