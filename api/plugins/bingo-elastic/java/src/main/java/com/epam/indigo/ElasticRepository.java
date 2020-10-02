package com.epam.indigo;

import org.apache.http.HttpHost;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestHighLevelClient;

import java.util.List;

public class ElasticRepository<T extends IndigoRecord> {


    public boolean createIndex() {
// need to get T and all annotationd in order to create index?
//        or instance of T
//        this.
    }

    private final RestHighLevelClient elasticClient;

//    todo intialise properly with auth, ssl, etc.
    public ElasticRepository(String hostname, int port, String scheme) {
        this.elasticClient = new RestHighLevelClient(
                RestClient.builder(
                        new HttpHost(hostname, port, scheme)));

    }

    List<T> fetch(int offset, int limit) {
        return null;
    }

}
