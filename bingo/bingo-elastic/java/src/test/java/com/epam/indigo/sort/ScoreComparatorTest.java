package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.ScoreSortBuilder;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;
import org.junit.jupiter.api.Test;

import java.util.Collection;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertNotNull;

public class ScoreComparatorTest {

    @Test
    public void testDefaultConstructor() {
        ScoreComparator<IndigoRecord> comparator = new ScoreComparator<>();

        assertEquals(SortOrder.DESC, comparator.getSortOrder());
    }

    @Test
    public void testParameterizedConstructor() {
        SortOrder sortOrder = SortOrder.ASC;
        ScoreComparator<IndigoRecord> comparator = new ScoreComparator<>(sortOrder);
        assertEquals(sortOrder, comparator.getSortOrder());
    }

    @Test
    public void testToSortBuilders() {
        SortOrder sortOrder = SortOrder.DESC;
        ScoreComparator<IndigoRecord> comparator = new ScoreComparator<>(sortOrder);

        Collection<SortBuilder<?>> sortBuilders = comparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(1, sortBuilders.size());

        SortBuilder<?> sortBuilder = sortBuilders.iterator().next();
        assertNotNull(sortBuilder);
        assertInstanceOf(ScoreSortBuilder.class, sortBuilder);

        ScoreSortBuilder scoreSortBuilder = (ScoreSortBuilder) sortBuilder;
        assertEquals(sortOrder, scoreSortBuilder.order());
    }

    @Test
    public void testBuilderWithSortOrder() {
        SortOrder sortOrder = SortOrder.ASC;
        ScoreComparator.Builder<IndigoRecord> builder = ScoreComparator.builder();
        builder.withSortOrder(sortOrder);
        ScoreComparator<IndigoRecord> comparator = builder.build();

        assertEquals(sortOrder, comparator.getSortOrder());
    }

    @Test
    public void testBuilderCompleteFlow() {
        SortOrder sortOrder = SortOrder.ASC;
        ScoreComparator<IndigoRecord> comparator = ScoreComparator.builder()
                .withSortOrder(sortOrder)
                .build();

        assertNotNull(comparator);
        assertEquals(sortOrder, comparator.getSortOrder());

        Collection<SortBuilder<?>> sortBuilders = comparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(1, sortBuilders.size());

        SortBuilder<?> sortBuilder = sortBuilders.iterator().next();
        assertNotNull(sortBuilder);
        assertInstanceOf(ScoreSortBuilder.class, sortBuilder);

        ScoreSortBuilder scoreSortBuilder = (ScoreSortBuilder) sortBuilder;
        assertEquals(sortOrder, scoreSortBuilder.order());
    }
}

