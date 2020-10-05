package com.epam.indigo.elastic;

import com.epam.indigo.model.IndigoRecord;
import org.apache.http.HttpHost;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestHighLevelClient;

import java.util.stream.Stream;

public class ElasticRepository<T extends IndigoRecord> {


    public ElasticRepository() {

        elasticClient = null;
    }

    public boolean createIndex() {
// need to get T and all annotationd in order to create index?
//        or instance of T
//        this.
        return false;
    }

    private final RestHighLevelClient elasticClient;

    //    todo intialise properly with auth, ssl, etc.
    public ElasticRepository(String hostname, int port, String scheme) {
        this.elasticClient = new RestHighLevelClient(
                RestClient.builder(
                        new HttpHost(hostname, port, scheme)));

    }

    public Stream<T> stream() {
        return null;
    }
}
