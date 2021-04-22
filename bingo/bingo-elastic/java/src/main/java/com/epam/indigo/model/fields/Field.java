package com.epam.indigo.model.fields;

final public class Field {

    protected final Object field;

    public Field(Object object) {
        field = object;
    }

    @Override
    public String toString() {
        return field.toString();
    }
}
