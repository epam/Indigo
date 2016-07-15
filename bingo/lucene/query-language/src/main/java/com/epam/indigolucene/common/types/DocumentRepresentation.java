package com.epam.indigolucene.common.types;

import java.util.Map;

/**
 * Created by Artem Malykh on 28.03.16.
 */
public interface DocumentRepresentation<S> {
    Map<String, Object> fieldsMap();
}
