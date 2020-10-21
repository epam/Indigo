package com.epam.indigo.model.fields;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;

public class Cmf {

    byte[] cmf;

    public Cmf(byte[] cmf) {
        this.cmf = cmf;
    }

    public Cmf(Object cmf) {
        try {
            this.cmf = convertToByteArray(cmf);
        } catch (IOException e) {
            // TODO: throw exception to the next level?
            System.out.println(e);
            this.cmf = new byte[0];
        }
    }

    protected byte[] convertToByteArray(Object cmf) throws IOException {
        try (ByteArrayOutputStream bos = new ByteArrayOutputStream(); ObjectOutput out = new ObjectOutputStream(bos)) {
            out.writeObject(cmf);
            return bos.toByteArray();
        }
    }

    public byte[] toByteArray() {
        return cmf;
    }

}
