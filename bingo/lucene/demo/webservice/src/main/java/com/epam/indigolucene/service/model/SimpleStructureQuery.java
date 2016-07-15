package com.epam.indigolucene.service.model;

/**
 * Created by Artem Malykh on 21.06.16.
 */
public class SimpleStructureQuery {
    protected int offset;
    protected int limit;
    protected String structure;

    public SimpleStructureQuery() {

    }

    public int getOffset() {
        return offset;
    }

    public void setOffset(int offset) {
        this.offset = offset;
    }

    public int getLimit() {
        return limit;
    }

    public void setLimit(int limit) {
        this.limit = limit;
    }

    public String getStructure() {
        return structure;
    }

    public void setStructure(String structure) {
        this.structure = structure;
    }
}
