package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import com.epam.indigo.predicate.BaseMatch;
import com.epam.indigo.predicate.ExactMatch;
import com.epam.indigo.predicate.FilterPredicate;
import com.epam.indigo.predicate.IndigoPredicate;
import com.epam.indigo.predicate.SubstructureMatch;
import com.epam.indigo.sort.IndigoComparator;
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
import org.elasticsearch.search.sort.FieldSortBuilder;
import org.elasticsearch.search.sort.SortOrder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Spliterator;
import java.util.function.*;
import java.util.stream.Collector;
import java.util.stream.DoubleStream;
import java.util.stream.IntStream;
import java.util.stream.LongStream;
import java.util.stream.Stream;

import static com.epam.indigo.model.NamingConstants.SIM_FINGERPRINT;
import static com.epam.indigo.model.NamingConstants.SIM_FINGERPRINT_LEN;
import static com.epam.indigo.model.NamingConstants.SUB_FINGERPRINT;
import static com.epam.indigo.model.NamingConstants.SUB_FINGERPRINT_LEN;

/**
 * Implementation of JDK Stream API
 * Limited number of operations supported at the moment, check out usage example in README for better understanding
 */
public class ElasticStream<T extends IndigoRecord> implements Stream<T> {

    private final RestHighLevelClient elasticClient;
    private final List<IndigoPredicate<? super T>> predicates = new ArrayList<>();
    private final String indexName;
    private int limit = Integer.MAX_VALUE;
    private final List<IndigoComparator<? super T>> comparators = new ArrayList<>();

    private static final int BATCH_SIZE = 10000;

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
        if (maxSize > Integer.MAX_VALUE)
            throw new IllegalArgumentException(String.format("Bingo Elastic max page size should be less than or equal to %1$d", Integer.MAX_VALUE));
        this.limit = (int) maxSize;
        return this;
    }

    @Override
    public boolean isParallel() {
        return false;
    }

    @Override
    public <R, A> R collect(Collector<? super T, A, R> collector) {
        A container = collector.supplier().get();
        Object[] searchAfterParameters = null;

        long processedRecords = 0;
        boolean continueSearch = true;

        while (continueSearch) {
            int currentBatchSize = (int) Math.min(BATCH_SIZE, limit - processedRecords);
            SearchRequest searchRequest = compileRequest(searchAfterParameters, currentBatchSize);
            SearchResponse searchResponse;
            try {
                searchResponse = elasticClient.search(searchRequest, RequestOptions.DEFAULT);
            } catch (IOException e) {
                throw new BingoElasticException("Couldn't complete search in Elasticsearch", e);
            }

            SearchHit[] hits = searchResponse.getHits().getHits();
            if (hits.length == 0) {
                break;
            }

            for (SearchHit hit : hits) {
                if (processedRecords >= limit) {
                    break;
                }
                T record = convertHitToRecord(hit);
                collector.accumulator().accept(container, record);
                processedRecords++;
            }

            searchAfterParameters = hits[hits.length - 1].getSortValues();
            continueSearch = !this.comparators.isEmpty() && hits.length == BATCH_SIZE;
        }

        return collector.finisher().apply(container);
    }


    private T convertHitToRecord(SearchHit hit) {
        if (NamingConstants.BINGO_REACTIONS.equals(this.indexName)) {
            return (T) Helpers.reactionFromElastic(hit.getId(), hit.getSourceAsMap(), hit.getScore());
        } else if (NamingConstants.BINGO_MOLECULES.equals(this.indexName)) {
            return (T) Helpers.moleculeFromElastic(hit.getId(), hit.getSourceAsMap(), hit.getScore());
        } else {
            throw new BingoElasticException("Unsupported index " + this.indexName);
        }
    }

    private QueryBuilder[] generateClauses(List<Integer> fingerprint, String field) {
        QueryBuilder[] bits = new QueryBuilder[fingerprint.size()];
        for (int i = 0; i < bits.length; ++i) {
            bits[i] = QueryBuilders.termQuery(field, fingerprint.get(i));
        }
        return bits;
    }

    private SearchRequest compileRequest(Object[] searchAfterParameters, int batchSize) {
        SearchRequest searchRequest = new SearchRequest(this.indexName);
        SearchSourceBuilder searchSourceBuilder = new SearchSourceBuilder();

        boolean similarityRequested = false;
        boolean isEmptyFingerprint = false;

        searchSourceBuilder.size(batchSize);

        if (!comparators.isEmpty()) {
            comparators.stream()
                    .flatMap(comparator -> comparator.toSortBuilders().stream())
                    .forEach(searchSourceBuilder::sort);
            searchSourceBuilder.sort(new FieldSortBuilder("_doc").order(SortOrder.ASC));
        }


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
            searchSourceBuilder.query(QueryBuilders.scriptScoreQuery(boolQueryBuilder, script));
        }

        if (searchAfterParameters != null) {
            searchSourceBuilder.searchAfter(searchAfterParameters);
        }

        searchRequest.source(searchSourceBuilder);
        return searchRequest;
    }

    @Override
    public ElasticStream<T> sorted(Comparator<? super T> comparator) {
        if (!(comparator instanceof IndigoComparator)) {
            throw new IllegalArgumentException("Comparator used isn't an IndigoComparator");
        }
        this.comparators.clear();
        this.comparators.add((IndigoComparator<? super T>) comparator);
        return this;
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
