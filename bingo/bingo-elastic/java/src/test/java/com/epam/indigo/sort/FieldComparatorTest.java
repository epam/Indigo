package com.epam.indigo.sort;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.search.sort.FieldSortBuilder;
import org.elasticsearch.search.sort.SortBuilder;
import org.elasticsearch.search.sort.SortOrder;
import org.junit.jupiter.api.Test;

import java.util.Collection;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertNotNull;


public class FieldComparatorTest {

    @Test
    public void testConstructor() {
        String fieldName = "testField";
        SortOrder sortOrder = SortOrder.ASC;
        FieldComparator<IndigoRecord> comparator = new FieldComparator<>(fieldName, sortOrder);

        assertEquals(fieldName, comparator.getFieldName());
        assertEquals(sortOrder, comparator.getSortOrder());
    }

    @Test
    public void testToSortBuilders() {
        String fieldName = "testField";
        SortOrder sortOrder = SortOrder.ASC;
        FieldComparator<IndigoRecord> comparator = new FieldComparator<>(fieldName, sortOrder);

        Collection<SortBuilder<?>> sortBuilders = comparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(1, sortBuilders.size());

        SortBuilder<?> sortBuilder = sortBuilders.iterator().next();
        assertNotNull(sortBuilder);
        assertInstanceOf(FieldSortBuilder.class, sortBuilder);

        FieldSortBuilder fieldSortBuilder = (FieldSortBuilder) sortBuilder;
        assertEquals(fieldName, fieldSortBuilder.getFieldName());
        assertEquals(sortOrder, fieldSortBuilder.order());
    }

    @Test
    public void testBuilderWithFieldName() {
        String fieldName = "testField";
        FieldComparator.Builder<IndigoRecord> builder = FieldComparator.builder();
        builder.withFieldName(fieldName);
        FieldComparator<IndigoRecord> comparator = builder.build();

        assertEquals(fieldName, comparator.getFieldName());
    }

    @Test
    public void testBuilderWithSortOrder() {
        SortOrder sortOrder = SortOrder.ASC;
        FieldComparator.Builder<IndigoRecord> builder = FieldComparator.builder();
        builder.withSortOrder(sortOrder);
        FieldComparator<IndigoRecord> comparator = builder.build();

        assertEquals(sortOrder, comparator.getSortOrder());
    }

    @Test
    public void testBuilderCompleteFlow() {
        String fieldName = "testField";
        SortOrder sortOrder = SortOrder.DESC;
        FieldComparator<IndigoRecord> comparator = FieldComparator.builder()
                .withFieldName(fieldName)
                .withSortOrder(sortOrder)
                .build();

        assertNotNull(comparator);
        assertEquals(fieldName, comparator.getFieldName());
        assertEquals(sortOrder, comparator.getSortOrder());

        Collection<SortBuilder<?>> sortBuilders = comparator.toSortBuilders();
        assertNotNull(sortBuilders);
        assertEquals(1, sortBuilders.size());

        SortBuilder<?> sortBuilder = sortBuilders.iterator().next();
        assertNotNull(sortBuilder);
        assertInstanceOf(FieldSortBuilder.class, sortBuilder);

        FieldSortBuilder fieldSortBuilder = (FieldSortBuilder) sortBuilder;
        assertEquals(fieldName, fieldSortBuilder.getFieldName());
        assertEquals(sortOrder, fieldSortBuilder.order());
    }
}
