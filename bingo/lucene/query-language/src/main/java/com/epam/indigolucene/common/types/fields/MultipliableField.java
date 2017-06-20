package com.epam.indigolucene.common.types.fields;

import java.util.List;
/**
 * Represents interface for fields which can have multiple values.
 *
 * @author Artem Malykh
 * created on 2016-04-01
 */
public interface MultipliableField<Vs, Vt> {
    Vt createValue(List<Vs> vals);
}
