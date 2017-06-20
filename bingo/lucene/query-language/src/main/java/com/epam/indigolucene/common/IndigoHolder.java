package com.epam.indigolucene.common;

import com.epam.indigo.Indigo;

/**
 * Indigo instance giver(if you ask very kindly).
 *
 * @author Artem Malykh
 * created on 2015-01-10
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
