package com.epam.indigolucene.common.exceptions;

/**
 * An exception class, useful on a stage of committing a document to a Solr server.
 *
 * @author Artem Malykh
 * created on 2016-03-29
 */
public class CommitException extends Exception {
    public CommitException(Exception e) {
        super(e);
    }
}
