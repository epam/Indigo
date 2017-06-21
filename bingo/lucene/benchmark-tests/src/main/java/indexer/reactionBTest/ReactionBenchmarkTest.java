package indexer.reactionBTest;

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
import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import static indexer.data.generated.TestSchema.REACT;

/**
 * The ReactionBenchmarkTest class contains benchmark tests for all currently available reaction structure
 * search methods.
 *
 * @author Filipp Pisarev
 * created on 21/06/2017
 */
public class ReactionBenchmarkTest {
    static {
        BasicConfigurator.configure();
        Logger.getRootLogger().setLevel(org.apache.log4j.Level.INFO);
    }

    @State(Scope.Thread)
    public static class SolrAndReactionsData {
        @Setup(Level.Trial)
        public void doSetup() throws Exception {
            SolrConnectionFactory.init(SolrConnection5.class);
            testCollection = TestSchema.collection(ServiceConfig.SERVICE_URL, CORE_NAME);
            try (SolrUploadStream ustream = testCollection.uploadStream()) {
                TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();
                emptyDocument.setContentType("reactValue");
                emptyDocument.setReact(IndigoHolder.getIndigo().loadReaction(REACTION));
                ustream.addDocument(emptyDocument);
            }
        }
        public CollectionRepresentation<TestSchema> testCollection;
        public List<Map<String, Object>> result;
        public final  String REACTION = "[I-].[Na+].C=CCBr>>[Na+].[Br-].C=CCI";
        public final String CORE_NAME = "moldocs";
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.SECONDS)
    @Fork(3)
    @Warmup(iterations=1)
    @Measurement(iterations=2)
    public void reactSimilaritySearchBenchmark(SolrAndReactionsData solrReactData, Blackhole blackhole) throws Exception {
        solrReactData.result = new LinkedList<>();
        solrReactData.testCollection.find().filter(REACT.unsafeIsSimilarTo(solrReactData.REACTION)).
                processWith(solrReactData.result::addAll);
        blackhole.consume(solrReactData.result);
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.SECONDS)
    @Fork(3)
    @Warmup(iterations=1)
    @Measurement(iterations=2)
    public void reactExactMatchSearchBenchmark(SolrAndReactionsData solrReactData, Blackhole blackhole) throws Exception {
        solrReactData.result = new LinkedList<>();
        solrReactData.testCollection.find().filter(REACT.unsafeExactMatches(solrReactData.REACTION)).
                processWith(solrReactData.result::addAll);
        blackhole.consume(solrReactData.result);
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.SECONDS)
    @Fork(3)
    @Warmup(iterations=1)
    @Measurement(iterations=2)
    public void reactSubstructureSearchBenchmarks(SolrAndReactionsData solrReactData, Blackhole blackhole) throws Exception {
        solrReactData.result = new LinkedList<>();
        solrReactData.testCollection.find().filter(REACT.unsafeHasSubstructure(solrReactData.REACTION)).
                processWith(solrReactData.result::addAll);
        blackhole.consume(solrReactData.result);
    }
}
