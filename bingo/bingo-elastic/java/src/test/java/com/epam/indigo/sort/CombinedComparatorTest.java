package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.FieldSortBuilder;
import org.elasticsearch.search.sort.ScoreSortBuilder;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertNotNull;

public class CombinedComparatorTest {

    @Test
    public void testToSortBuilders() {
        FieldComparator<IndigoRecord> fieldComparator = new FieldComparator<>("testField", SortOrder.ASC);
        ScoreComparator<IndigoRecord> scoreComparator = new ScoreComparator<>(SortOrder.DESC);

        CombinedComparator<IndigoRecord> combinedComparator = CombinedComparator.builder()
                .withComparator(fieldComparator)
                .withComparator(scoreComparator)
                .build();

        Collection<SortBuilder<?>> sortBuilders = combinedComparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(2, sortBuilders.size());

        List<SortBuilder<?>> sortBuilderList = (List<SortBuilder<?>>) sortBuilders;
        FieldSortBuilder fieldSortBuilder = (FieldSortBuilder) sortBuilderList.get(0);
        assertEquals("testField", fieldSortBuilder.getFieldName());
        assertEquals(SortOrder.ASC, fieldSortBuilder.order());

        assertInstanceOf(ScoreSortBuilder.class, sortBuilderList.get(1));
    }

    @Test
    public void testBuilderWithComparators() {
        FieldComparator<IndigoRecord> fieldComparator = new FieldComparator<>("testField1", SortOrder.ASC);
        ScoreComparator<IndigoRecord> scoreComparator = new ScoreComparator<>(SortOrder.DESC);

        List<IndigoComparator<IndigoRecord>> comparators = Arrays.asList(fieldComparator, scoreComparator);
        CombinedComparator<IndigoRecord> combinedComparator = CombinedComparator.builder()
                .withComparators(comparators)
                .build();

        Collection<SortBuilder<?>> sortBuilders = combinedComparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(2, sortBuilders.size());
    }
}

