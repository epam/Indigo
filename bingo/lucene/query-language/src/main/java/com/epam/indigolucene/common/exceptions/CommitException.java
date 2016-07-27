package com.epam.indigolucene.common.exceptions;

/**
 * Created by Artem Malykh on 29.03.16.
 */
public class CommitException extends Exception {
    public CommitException(Exception e) {
        super(e);
    }
}
