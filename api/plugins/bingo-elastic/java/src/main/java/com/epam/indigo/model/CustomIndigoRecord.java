package com.epam.indigo.model;

import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.annotation.RangeQuery;
import com.epam.indigo.model.annotation.WildcardQuery;

public class CustomIndigoRecord extends IndigoRecord {

    @RangeQuery
    private int moleculeWeight;

    @WildcardQuery
    private String customTag;


}
