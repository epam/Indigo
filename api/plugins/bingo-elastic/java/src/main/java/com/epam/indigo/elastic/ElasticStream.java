package com.epam.indigo.elastic;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.FilterPredicate;
import com.epam.indigo.predicate.IndigoPredicate;
import com.epam.indigo.predicate.SimilarityMatch;
import com.epam.indigo.predicate.TopNPredicate;
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

public class ElasticStream<T extends IndigoRecord> implements Stream<T> {

    private final RestHighLevelClient elasticClient;
    private int size = 10;
    private final List<IndigoPredicate<? super T>> predicates = new ArrayList<>();
    private final String indexName;

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
    public <R> Stream<R> map(Function<? super T, ? extends R> mapper) {
        return null;
    }

    @Override
    public IntStream mapToInt(ToIntFunction<? super T> mapper) {
        return null;
    }

    @Override
    public LongStream mapToLong(ToLongFunction<? super T> mapper) {
        return null;
    }

    @Override
    public DoubleStream mapToDouble(ToDoubleFunction<? super T> mapper) {
        return null;
    }

    @Override
    public <R> Stream<R> flatMap(Function<? super T, ? extends Stream<? extends R>> mapper) {
        return null;
    }

    @Override
    public IntStream flatMapToInt(Function<? super T, ? extends IntStream> mapper) {
        return null;
    }

    @Override
    public LongStream flatMapToLong(Function<? super T, ? extends LongStream> mapper) {
        return null;
    }

    @Override
    public DoubleStream flatMapToDouble(Function<? super T, ? extends DoubleStream> mapper) {
        return null;
    }

    @Override
    public Stream<T> distinct() {
        return null;
    }

    @Override
    public Stream<T> sorted() {
        return null;
    }

    @Override
    public Stream<T> sorted(Comparator<? super T> comparator) {
        return null;
    }

    @Override
    public Stream<T> peek(Consumer<? super T> action) {
        return null;
    }

    @Override
    public Stream<T> limit(long maxSize) {
        this.size = (int) maxSize;
        return this;
    }

    @Override
    public Stream<T> skip(long n) {
        return null;
    }

    @Override
    public void forEach(Consumer<? super T> action) {

    }

    @Override
    public void forEachOrdered(Consumer<? super T> action) {

    }

    @Override
    public Object[] toArray() {
        return new Object[0];
    }

    @Override
    public <A> A[] toArray(IntFunction<A[]> generator) {
        return null;
    }

    @Override
    public T reduce(T identity, BinaryOperator<T> accumulator) {
        return null;
    }

    @Override
    public Optional<T> reduce(BinaryOperator<T> accumulator) {
        return Optional.empty();
    }

    @Override
    public <U> U reduce(U identity, BiFunction<U, ? super T, U> accumulator, BinaryOperator<U> combiner) {
        return null;
    }

    @Override
    public <R> R collect(Supplier<R> supplier, BiConsumer<R, ? super T> accumulator, BiConsumer<R, R> combiner) {
        return null;
    }

    @Override
    public <R, A> R collect(Collector<? super T, A, R> collector) {
        A container = collector.supplier().get();
        SearchRequest searchRequest = new SearchRequest(this.indexName);
        SearchSourceBuilder searchSourceBuilder = new SearchSourceBuilder();
        boolean similarityRequested = false;
        if (this.predicates.isEmpty()) {
            searchSourceBuilder.query(QueryBuilders.matchAllQuery());
        } else {
            BoolQueryBuilder boolQueryBuilder = QueryBuilders.boolQuery();
            Script script = null;
            for (IndigoPredicate<? super T> predicate : this.predicates) {
                if (predicate instanceof TopNPredicate) {
                    searchSourceBuilder.size(((TopNPredicate<?>) predicate).getTopN());
                }
                if (predicate instanceof SimilarityMatch) {
                    if (!similarityRequested) {
                        similarityRequested = true;
                        QueryBuilder[] shouldClauses = generateShouldClauses(((SimilarityMatch<?>) predicate).getTarget());
                        for (QueryBuilder should : shouldClauses) {
                            boolQueryBuilder.should(should);
                        }
//                        TODO implement proper mm based on threshold ask
//                        boolQueryBuilder.minimumShouldMatch((int) (((SimilarityMatch<?>) predicate).getThreshold() * 100));
                        script = ((SimilarityMatch<?>) predicate).generateScript();
                    } else {
                        throw new IllegalArgumentException("Several similarity matches requested, couldn't process query");
                    }
                }
                if (predicate instanceof FilterPredicate) {
                    boolQueryBuilder.must(((FilterPredicate<?>) predicate).generateQuery());
                }
            }
            if (script == null) {
                script = generateIdentityScore();
            }
            searchSourceBuilder.size(this.size);
            searchSourceBuilder.query(QueryBuilders.scriptScoreQuery(boolQueryBuilder, script));
        }
        searchRequest.source(searchSourceBuilder);
        SearchHit[] hits = new SearchHit[0];
        try {
            SearchResponse searchResponse = this.elasticClient.search(searchRequest, RequestOptions.DEFAULT);
            hits = searchResponse.getHits().getHits();
        } catch (IOException e) {
//            TODO logging
            System.out.println(e);
        }
        for (SearchHit hit : hits) {
            collector.accumulator().accept(container, (T) Helpers.fromSource(hit.getId(), hit.getSourceAsMap(), hit.getScore()));
        }
        return (R) container;
    }


    @Override
    public Optional<T> min(Comparator<? super T> comparator) {
        return Optional.empty();
    }

    @Override
    public Optional<T> max(Comparator<? super T> comparator) {
        return Optional.empty();
    }

    @Override
    public long count() {
        return 0;
    }

    @Override
    public boolean anyMatch(Predicate<? super T> predicate) {
        return false;
    }

    @Override
    public boolean allMatch(Predicate<? super T> predicate) {
        return false;
    }

    @Override
    public boolean noneMatch(Predicate<? super T> predicate) {
        return false;
    }

    @Override
    public Optional<T> findFirst() {
        return Optional.empty();
    }

    @Override
    public Optional<T> findAny() {
        return Optional.empty();
    }

    @Override
    public Iterator<T> iterator() {
        return null;
    }

    @Override
    public Spliterator<T> spliterator() {
        return null;
    }

    @Override
    public boolean isParallel() {
//        no support to collect records in parallel for now
        return false;
    }

    @Override
    public Stream<T> sequential() {
        return null;
    }

    @Override
    public Stream<T> parallel() {
        return null;
    }

    @Override
    public Stream<T> unordered() {
        return null;
    }

    @Override
    public Stream<T> onClose(Runnable closeHandler) {
        return null;
    }

    @Override
    public void close() {

    }

    private QueryBuilder[] generateShouldClauses(IndigoRecord target) {
        List<Integer> fingerprint = target.getFingerprint();
        QueryBuilder[] bits = new QueryBuilder[fingerprint.size()];
        for (int i = 0; i < bits.length; ++i) {
            bits[i] = QueryBuilders.termQuery("fingerprint", fingerprint.get(i));
        }
        return bits;
    }

    private Script generateIdentityScore() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score");
        return Script.parse(map);
    }
}
