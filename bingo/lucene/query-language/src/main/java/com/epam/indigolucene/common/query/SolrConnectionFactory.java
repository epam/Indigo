package com.epam.indigolucene.common.query;

import java.io.IOException;

/**
 * A factory pattern. This class provides an instance of SolrConnection along with connection class initialization.
 *
 * @author Artem Malykh
 * created on 2016-03-23
 */
public class SolrConnectionFactory {
    private static SolrConnection instance;
    private static Class<? extends SolrConnection> clazz;

    public static  <T extends SolrConnection> void init(Class<T> clazz) {
        if (SolrConnectionFactory.clazz == null) {
            synchronized (SolrConnectionFactory.class) {
                if (SolrConnectionFactory.clazz == null) {
                    SolrConnectionFactory.clazz = clazz;
                }
            }
        }
        if (!clazz.equals(SolrConnectionFactory.clazz)) {
            throw new ExceptionInInitializerError("Factory is already initialized");
        }
    }

    public static SolrConnection createInstance() throws IllegalAccessException, InstantiationException {
        if (instance == null) {
            synchronized (SolrConnectionFactory.class) {
                if (instance == null) {
                    instance = clazz.newInstance();
                }
            }
        }
        return instance;
    }

    public static void clear() {
        try {
            if (instance != null)
                instance.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            SolrConnectionFactory.clazz = null;
            instance = null;
        }
    }
}
