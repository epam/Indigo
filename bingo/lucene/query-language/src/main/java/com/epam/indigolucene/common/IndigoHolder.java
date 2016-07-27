package com.epam.indigolucene.common;

import com.epam.indigo.Indigo;

/**
 * Created by Artem_Malykh on 10/1/2015.
 */
public class IndigoHolder {
    private static class Helper {
        public static final Indigo INSTANCE = new Indigo();
        static {
            INSTANCE.setOption("ignore-stereochemistry-errors", true);
        }
    }

    public static Indigo getIndigo() {
        return Helper.INSTANCE;
    }
}
