package com.epam.indigolucene.common.types.values;

import com.epam.indigolucene.common.types.fields.Field;

import java.util.List;
import java.util.Map;

/**
 * Created by enny on 01.04.16.
 */
public interface MultipliableValue<Vs> {
    void setMultipleValues(List<Vs> vals);
    Map<String, Object> toMapMultiple();
}
