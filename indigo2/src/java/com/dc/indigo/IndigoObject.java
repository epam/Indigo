package com.dc.indigo.molecule
import java.lang.ref.Cleaner;

public class IndigoObject {
    private static Cleaner cleaner = Cleaner.create();
	private long handle;
    protected IndigoObject(long handle) {
        this.handle = handle;
        cleaner.register(this, () -> {
            lib_IndigoObject_release(handle);
        });
    }
    private native void lib_IndigoObject_release(long handle);
}
 