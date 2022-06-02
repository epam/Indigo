package com.epam.indigo;

public enum Hybridization {
    S(1, "S"),
    SP(2, "SP"),
    SP2(3, "SP2"),
    SP3(4, "SP3"),
    SP3D(5, "SP3D"),
    SP3D2(6, "SP3D2"),
    SP3D3(7, "SP3D3"),
    SP3D4(8, "SP3D4"),
    SP2D(9, "SP2D");

    private int numVal;
    private String strVal;

    Hybridization(int numVal, String strVal) {
        this.numVal = numVal;
        this.strVal = strVal;
    }

    public int getNumVal() {
        return numVal;
    }

    public String getStrVal() {
        return strVal;
    }


    public static Hybridization fromNum(int value) {
        switch (value) {
            case 1:
                return Hybridization.S;
            case 2:
                return Hybridization.SP;
            case 3:
                return Hybridization.SP2;
            case 4:
                return Hybridization.SP3;
            case 5:
                return Hybridization.SP3D;
            case 6:
                return Hybridization.SP3D2;
            case 7:
                return Hybridization.SP3D3;
            case 8:
                return Hybridization.SP3D4;
            case 9:
                return Hybridization.SP2D;
            default:
                throw new IndigoException(new Object(), "Unknown hybridization");
        }
    }
}
