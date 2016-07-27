package com.epam.indigolucene.common.exceptions;

import java.util.Map;

/**
 * Created by Artem Malykh on 29.03.16.
 */
public class DocumentAdditionException extends Exception {
    public DocumentAdditionException(Map<String, Object> fieldsMap, Throwable cause) {
        super("Error while adding document constructed from map " + fieldsMap, cause);
    }
}
