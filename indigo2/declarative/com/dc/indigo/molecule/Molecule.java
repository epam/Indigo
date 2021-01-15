package com.dc.indigo.molecule


public interface Molecule {
    @Static // Meta-information is provided via annotations.
    Molecule createFromMolFile(String string);
    
    void aromatize();
    
    String molfile();
}

