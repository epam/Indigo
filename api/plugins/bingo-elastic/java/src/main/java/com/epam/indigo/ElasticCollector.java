package com.epam.indigo;

import java.util.Set;
import java.util.function.BiConsumer;
import java.util.function.BinaryOperator;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collector;

public class ElasticCollector
        implements Collector<Integer, A, R> {


    @Override
    public Supplier<A> supplier() {
        return null;
    }

    @Override
    public BiConsumer<A, Integer> accumulator() {
        return null;
    }

    @Override
    public BinaryOperator<A> combiner() {
        return null;
    }

    @Override
    public Function<A, R> finisher() {
        return null;
    }

    @Override
    public Set<Characteristics> characteristics() {
        return null;
    }
}
