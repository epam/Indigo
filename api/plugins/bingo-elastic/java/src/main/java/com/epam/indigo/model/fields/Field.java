package com.epam.indigo.model.fields;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.nio.charset.StandardCharsets;

final public class Field implements FieldInterface {

    protected Object field;


    public Field(Object object) {
        field = object;
    }


    public byte[] toByteArray() {
        try {
            try (ByteArrayOutputStream bos = new ByteArrayOutputStream(); ObjectOutput out = new ObjectOutputStream(bos)) {
                out.writeObject(field);
                return bos.toByteArray();
            }
        } catch (IOException e) {
            return new byte[0];
        }
    }

    public String toString() {
        return field.toString();
    }

}
