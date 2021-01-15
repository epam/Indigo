package com.dc.indigo.molecule


public class MoleculeGeneratedImpl extends IndigoObject implements Molecule {
    static Molecule createFromMolFile(String string) {
        return new MoleculeGeneratedImpl(lib_createFromMolFile(string));
    }

    protected MoleculeGeneratedImpl(long handle) {
        super(handle);
    }

    @Override
    public Molecule createFromMolFile(String string) {
        return MoleculeGeneratedImpl.createFromMolFile(string);
    }
    
    @Override
    public void aromatize() {
        lib_aromatize(handle);
    }
    
    @Override
    public String molfile() {
        return lib_molfile(handle);
    }

    private native lib_createFromMolFile(String string)
    private native lib_aromatize(long handle);
    private native lib_molfile(long handle);
}

