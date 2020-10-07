package com.epam.indigo.model;

import com.epam.indigo.model.annotation.NestedObject;
import com.epam.indigo.model.annotation.RangeQuery;
import com.epam.indigo.model.annotation.WildcardQuery;

import java.util.List;

public class CustomIndigoRecord extends IndigoRecord {

    @RangeQuery
    private int moleculeWeight;

    @WildcardQuery
    private String customTag;

    @NestedObject
    private List<Integer> ints;

    public int getMoleculeWeight() {
        return moleculeWeight;
    }

    public void setMoleculeWeight(int moleculeWeight) {
        this.moleculeWeight = moleculeWeight;
    }

    public String getCustomTag() {
        return customTag;
    }

    public void setCustomTag(String customTag) {
        this.customTag = customTag;
    }

    //

    // todo create objects from sdf, mol, ... ?


}
