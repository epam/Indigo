package com.epam.indigolucene.common.types.fields;

import java.util.List;

/**
 * Created by Artem Malykh on 01.04.16.
 * Represents interface for fields which can have multiple values.
 */
public interface MultipliableField<Vs, Vt> {
    Vt createValue(List<Vs> vals);
}
