package com.epam.indigolucene.common.query;

/**
 * Created by Artem Malykh on 23.03.16.
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
}
