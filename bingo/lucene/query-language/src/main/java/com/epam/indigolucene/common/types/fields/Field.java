package com.epam.indigolucene.common.types.fields;
/**
 * A basis class for any field representation of Solr's schema.xml. Widely used for extension in a different kinds of
 * more specific fields.
 *
 * @author Artem Malykh
 * created on 2016-02-20
 */
public abstract class Field<S, Vs, Vt> {
    protected String name;

    protected boolean isMultiple;

    public Field(String name, boolean isMultiple) {
        this.name = name;
        this.isMultiple = isMultiple;
    }

    public String getName() {
        return name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Field field = (Field) o;
        return name != null ? name.equals(field.name) : field.name == null;
    }

    @Override
    public int hashCode() {
        return name != null ? name.hashCode() : 0;
    }

    public abstract Vt createValue(Vs from);

    public boolean isMultiple() {
        return isMultiple;
    }
}
