package com.epam.indigo.model.fields;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;

final public class Field implements FieldInterface {

    protected byte[] field;

    public Field(byte[] field) {
        this.field = field;
    }

    public Field(Object object) {
        try {
            this.field = convertToByteArray(object);
        } catch (IOException e) {
            // TODO: throw exception to the next level?
            System.out.println(e);
            this.field = new byte[0];
        }
    }

    final protected byte[] convertToByteArray(Object object) throws IOException {
        try (ByteArrayOutputStream bos = new ByteArrayOutputStream(); ObjectOutput out = new ObjectOutputStream(bos)) {
            out.writeObject(object);
            return bos.toByteArray();
        }
    }

    public byte[] toByteArray() {
        return field;
    }

    public String toString() {
        return new String(field);
    }

}
