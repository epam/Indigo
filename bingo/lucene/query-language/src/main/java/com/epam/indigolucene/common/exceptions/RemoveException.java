package com.epam.indigolucene.common.exceptions;

/**
 * An exception class, useful on a stage of removal of a document from a Solr server.
 *
 * @author Artem Malykh
 * created on 2016-03-31
 */
public class RemoveException extends Exception {
    public RemoveException(Throwable cause) {
        super(cause);
    }
}
