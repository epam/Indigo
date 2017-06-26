package com.epam.indigolucene.common.types.values;

import com.epam.indigolucene.common.types.fields.Field;

import java.util.List;
import java.util.Map;

/**
 * MultipliableValue class maintain a value of a structures, which can have multiple values along with "toMap" method.
 *
 * @author enny
 * created on 2016-04-01
 */
public interface MultipliableValue<Vs> {
    void setMultipleValues(List<Vs> vals);
    Map<String, Object> toMapMultiple();
}
