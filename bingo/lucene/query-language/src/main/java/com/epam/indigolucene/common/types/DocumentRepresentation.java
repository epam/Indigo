package com.epam.indigolucene.common.types;

import java.util.Map;
/**
 * Solr's document fields representation interface.
 *
 * @author Artem Malykh
 * created on 2016-03-28
 */
@FunctionalInterface
public interface DocumentRepresentation<S> {
    Map<String, Object> fieldsMap();
}
