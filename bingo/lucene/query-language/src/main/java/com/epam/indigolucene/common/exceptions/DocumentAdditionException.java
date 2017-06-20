package com.epam.indigolucene.common.exceptions;

import java.util.Map;

/**
 * An exception class, useful on a stage of addition of a document to a Solr server.
 *
 * @author Artem Malykh
 * created on 2016-03-29
 */
public class DocumentAdditionException extends Exception {
    public DocumentAdditionException(Map<String, Object> fieldsMap, Throwable cause) {
        super("Error while adding document constructed from map " + fieldsMap, cause);
    }
}
