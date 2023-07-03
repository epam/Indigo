package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import com.epam.indigo.predicate.*;
import org.elasticsearch.action.search.SearchRequest;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.RequestOptions;
import org.elasticsearch.client.RestHighLevelClient;
import org.elasticsearch.index.query.BoolQueryBuilder;
import org.elasticsearch.index.query.QueryBuilder;
import org.elasticsearch.index.query.QueryBuilders;
import org.elasticsearch.script.Script;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.builder.SearchSourceBuilder;

import java.io.IOException;
import java.util.*;
import java.util.function.*;
import java.util.stream.*;

import static com.epam.indigo.model.NamingConstants.*;

/**
 * Implementation of JDK Stream API
 * Limited number of operations supported at the moment, check out usage example in README for better understanding
 */
public class ElasticStream<T extends IndigoRecord> implements Stream<T> {

    private final RestHighLevelClient elasticClient;
    private final List<IndigoPredicate<? super T>> predicates = new ArrayList<>();
    private final String indexName;
    private int size = 10;
    private final int MAX_ALLOWED_SIZE = 1000;

    public ElasticStream(RestHighLevelClient elasticClient, String indexName) {
        this.elasticClient = elasticClient;
        this.indexName = indexName;
    }

    @Override
    public Stream<T> filter(Predicate<? super T> predicate) {
        if (!(predicate instanceof IndigoPredicate)) {
            throw new IllegalArgumentException("Predicate used isn't an IndigoPredicate");
        }
        predicates.add((IndigoPredicate<? super T>) predicate);
        return this;
    }

    @Override
    public Stream<T> limit(long maxSize) {
        if (maxSize > MAX_ALLOWED_SIZE)
            throw new IllegalArgumentException(String.format("Bingo Elastic max page size should be less than or equal to %1", MAX_ALLOWED_SIZE));
        this.size = (int) maxSize;
        return this;
    }

    @Override
    public boolean isParallel() {
        return false;
    }

    @Override
    public <R, A> R collect(Collector<? super T, A, R> collector) {
        A container = collector.supplier().get();
        SearchRequest searchRequest = compileRequest();
        SearchHit[] hits;
        try {
            SearchResponse searchResponse = this.elasticClient.search(searchRequest, RequestOptions.DEFAULT);
            hits = searchResponse.getHits().getHits();
            if (NamingConstants.BINGO_REACTIONS.equals(this.indexName)) {
                for (SearchHit hit : hits) {
                    collector.accumulator().accept(container, (T) Helpers.reactionFromElastic(hit.getId(), hit.getSourceAsMap(), hit.getScore()));
                }
            } else if (NamingConstants.BINGO_MOLECULES.equals(this.indexName)) {
                for (SearchHit hit : hits) {
                    collector.accumulator().accept(container, (T) Helpers.moleculeFromElastic(hit.getId(), hit.getSourceAsMap(), hit.getScore()));
                }
            } else {
                throw new BingoElasticException("Unsupported index " + this.indexName);
            }
        } catch (IOException e) {
            throw new BingoElasticException("Couldn't complete search in Elasticsearch", e.getCause());
        }
        return (R) container;
    }

    private QueryBuilder[] generateClauses(List<Integer> fingerprint, String field) {
        QueryBuilder[] bits = new QueryBuilder[fingerprint.size()];
        for (int i = 0; i < bits.length; ++i) {
            bits[i] = QueryBuilders.termQuery(field, fingerprint.get(i));
        }
        return bits;
    }

    private SearchRequest compileRequest() {
        SearchRequest searchRequest = new SearchRequest(this.indexName);
        SearchSourceBuilder searchSourceBuilder = new SearchSourceBuilder();
        boolean similarityRequested = false;
        boolean isEmptyFingerprint = false;
        if (this.predicates.isEmpty()) {
            searchSourceBuilder.query(QueryBuilders.matchAllQuery());
        } else {
            BoolQueryBuilder boolQueryBuilder = QueryBuilders.boolQuery();
            Script script = null;
            float threshold = 0.0f;
            for (IndigoPredicate<? super T> predicate : this.predicates) {
                if (predicate instanceof BaseMatch) {
                    if (similarityRequested)
                        throw new BingoElasticException("Several similarity matches requested, couldn't create query");
                    similarityRequested = true;
                    if (predicate instanceof ExactMatch || predicate instanceof SubstructureMatch) {
                        QueryBuilder[] clauses = generateClauses(((BaseMatch<? super T>) predicate).getTarget().getSubFingerprint(), ((BaseMatch<? super T>) predicate).getFingerprintName());
                        if (clauses.length == 0)
                            isEmptyFingerprint = true;
                        for (QueryBuilder clause : clauses) {
                            boolQueryBuilder.must(clause);
                        }
                        boolQueryBuilder.must(QueryBuilders.termQuery(SUB_FINGERPRINT_LEN, clauses.length).boost(0.0f));
                    } else {
                        QueryBuilder[] clauses = generateClauses(((BaseMatch<? super T>) predicate).getTarget().getSimFingerprint(), ((BaseMatch<? super T>) predicate).getFingerprintName());
                        if (clauses.length == 0)
                            isEmptyFingerprint = true;
                        for (QueryBuilder clause : clauses) {
                            boolQueryBuilder.should(clause);
                        }
                        boolQueryBuilder.minimumShouldMatch(((BaseMatch<?>) predicate).getMinimumShouldMatch(clauses.length));
                    }
                    threshold = ((BaseMatch<? super T>) predicate).getThreshold();
                    script = ((BaseMatch<?>) predicate).generateScript();
                }

                if (predicate instanceof FilterPredicate) {
                    boolQueryBuilder.must(((FilterPredicate<?>) predicate).generateQuery());
                }
            }
            if (script == null || isEmptyFingerprint) {
                script = generateIdentityScore();
            }
            searchSourceBuilder.fetchSource(new String[]{"*"}, new String[]{SIM_FINGERPRINT, SIM_FINGERPRINT_LEN, SUB_FINGERPRINT_LEN, SUB_FINGERPRINT});
            searchSourceBuilder.minScore(threshold);
            searchSourceBuilder.size(this.size);
            searchSourceBuilder.query(QueryBuilders.scriptScoreQuery(boolQueryBuilder, script));
        }
        searchRequest.source(searchSourceBuilder);
        return searchRequest;
    }

    private Script generateIdentityScore() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score");
        return Script.parse(map);
    }


    @Override
    public Optional<T> min(Comparator<? super T> comparator) {
        throw new BingoElasticException("min() operation on this stream isn't implemented");
    }

    @Override
    public Optional<T> max(Comparator<? super T> comparator) {
        throw new BingoElasticException("max() operation on this stream isn't implemented");
    }

    @Override
    public long count() {
        throw new BingoElasticException("count() operation on this stream isn't implemented");
    }

    @Override
    public boolean anyMatch(Predicate<? super T> predicate) {
        throw new BingoElasticException("anyMatch() operation on this stream isn't implemented");
    }

    @Override
    public boolean allMatch(Predicate<? super T> predicate) {
        throw new BingoElasticException("allMatch() operation on this stream isn't implemented");
    }

    @Override
    public boolean noneMatch(Predicate<? super T> predicate) {
        throw new BingoElasticException("noneMatch() operation on this stream isn't implemented");
    }

    @Override
    public Optional<T> findFirst() {
        throw new BingoElasticException("findFirst() operation on this stream isn't implemented");
    }

    @Override
    public Optional<T> findAny() {
        throw new BingoElasticException("findAny() operation on this stream isn't implemented");
    }

    @Override
    public Iterator<T> iterator() {
        throw new BingoElasticException("iterator() operation on this stream isn't implemented");
    }

    @Override
    public Spliterator<T> spliterator() {
        throw new BingoElasticException("spliterator() operation on this stream isn't implemented");
    }

    @Override
    public <R> Stream<R> map(Function<? super T, ? extends R> mapper) {
        throw new BingoElasticException("map() operation on this stream isn't implemented");
    }

    @Override
    public IntStream mapToInt(ToIntFunction<? super T> mapper) {
        throw new BingoElasticException("mapToInt() operation on this stream isn't implemented");
    }

    @Override
    public LongStream mapToLong(ToLongFunction<? super T> mapper) {
        throw new BingoElasticException("mapToLong() operation on this stream isn't implemented");
    }

    @Override
    public DoubleStream mapToDouble(ToDoubleFunction<? super T> mapper) {
        throw new BingoElasticException("mapToDouble() operation on this stream isn't implemented");
    }

    @Override
    public <R> Stream<R> flatMap(Function<? super T, ? extends Stream<? extends R>> mapper) {
        throw new BingoElasticException("flatMap() operation on this stream isn't implemented");
    }

    @Override
    public IntStream flatMapToInt(Function<? super T, ? extends IntStream> mapper) {
        throw new BingoElasticException("flatMapToInt() operation on this stream isn't implemented");
    }

    @Override
    public LongStream flatMapToLong(Function<? super T, ? extends LongStream> mapper) {
        throw new BingoElasticException("flatMapToLong() operation on this stream isn't implemented");
    }

    @Override
    public DoubleStream flatMapToDouble(Function<? super T, ? extends DoubleStream> mapper) {
        throw new BingoElasticException("flatMapToDouble() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> distinct() {
        throw new BingoElasticException("distinct() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> sorted() {
        throw new BingoElasticException("sorted() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> sorted(Comparator<? super T> comparator) {
        throw new BingoElasticException("sorted() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> peek(Consumer<? super T> action) {
        throw new BingoElasticException("peek() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> skip(long n) {
        throw new BingoElasticException("skip() operation on this stream isn't implemented");
    }

    @Override
    public void forEach(Consumer<? super T> action) {
        throw new BingoElasticException("forEach() operation on this stream isn't implemented");
    }

    @Override
    public void forEachOrdered(Consumer<? super T> action) {
        throw new BingoElasticException("forEachOrdered() operation on this stream isn't implemented");
    }

    @Override
    public Object[] toArray() {
        throw new BingoElasticException("toArray() operation on this stream isn't implemented");
    }

    @Override
    public <A> A[] toArray(IntFunction<A[]> generator) {
        throw new BingoElasticException("toArray() operation on this stream isn't implemented");
    }

    @Override
    public T reduce(T identity, BinaryOperator<T> accumulator) {
        throw new BingoElasticException("reduce() operation on this stream isn't implemented");
    }

    @Override
    public Optional<T> reduce(BinaryOperator<T> accumulator) {
        throw new BingoElasticException("reduce() operation on this stream isn't implemented");
    }

    @Override
    public <U> U reduce(U identity, BiFunction<U, ? super T, U> accumulator, BinaryOperator<U> combiner) {
        throw new BingoElasticException("reduce() operation on this stream isn't implemented");
    }

    @Override
    public <R> R collect(Supplier<R> supplier, BiConsumer<R, ? super T> accumulator, BiConsumer<R, R> combiner) {
        throw new BingoElasticException("collect() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> sequential() {
        throw new BingoElasticException("sequential() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> parallel() {
        throw new BingoElasticException("parallel() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> unordered() {
        throw new BingoElasticException("unordered() operation on this stream isn't implemented");
    }

    @Override
    public Stream<T> onClose(Runnable closeHandler) {
        throw new BingoElasticException("onClose() operation on this stream isn't implemented");
    }

    @Override
    public void close() {
        throw new BingoElasticException("close() operation on this stream isn't implemented");
    }
}
