package com.epam.indigo.elastic;

import com.epam.indigo.GenericRepository;
import com.epam.indigo.model.IndigoRecord;
import org.apache.http.HttpHost;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.bulk.BulkRequest;
import org.elasticsearch.action.bulk.BulkResponse;
import org.elasticsearch.action.index.IndexRequest;
import org.elasticsearch.client.RequestOptions;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestClientBuilder;
import org.elasticsearch.client.RestHighLevelClient;
import org.elasticsearch.client.indices.CreateIndexRequest;
import org.elasticsearch.client.indices.CreateIndexResponse;
import org.elasticsearch.client.indices.GetIndexRequest;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentFactory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;
import java.util.stream.Stream;

public class ElasticRepository<T extends IndigoRecord> implements GenericRepository<T> {

    private String indexName;
    private String hostName;
    private int port;
    private String scheme;
    private String userName;
    private String password;
    private RestHighLevelClient elasticClient;
    private boolean ignoreSSL;

    private ElasticRepository() {
    }

    public static class ElasticRepositoryBuilder<T extends IndigoRecord> {
        private List<Consumer<ElasticRepository>> operations;

        public ElasticRepositoryBuilder() {
            this.operations = new ArrayList<>();
        }

        public ElasticRepositoryBuilder withIndexName(String indexName) {
            operations.add(repo -> repo.indexName = indexName);
            return this;
        }

        public ElasticRepositoryBuilder withHostName(String hostName) {
            operations.add(repo -> repo.hostName = hostName);
            return this;
        }

        public ElasticRepositoryBuilder withPort(Integer port) {
            operations.add(repo -> repo.port = port);
            return this;
        }

        public ElasticRepositoryBuilder withScheme(String scheme) {
            operations.add(repo -> repo.scheme = scheme);
            return this;
        }

        public ElasticRepositoryBuilder withUserName(String userName) {
            operations.add(repo -> repo.userName = userName);
            return this;
        }

        public ElasticRepositoryBuilder withPassword(String password) {
            operations.add(repo -> repo.password = password);
            return this;
        }

        public ElasticRepositoryBuilder withRestClient(RestHighLevelClient restHighLevelClient) {
            operations.add(repo -> repo.elasticClient = restHighLevelClient);
            return this;
        }

        public ElasticRepositoryBuilder withIgnoreSSL(boolean ignoreSSL) {
            operations.add(repo -> repo.ignoreSSL = ignoreSSL);
            return this;
        }

        public ElasticRepository<T> build() {
            ElasticRepository<T> repository = new ElasticRepository<>();
            operations.forEach(operation -> operation.accept(repository));
            if (repository.elasticClient == null) {
                RestClientBuilder builder = RestClient.builder(new HttpHost(repository.hostName, repository.port, repository.scheme));
                if (repository.userName != null && repository.password != null) {
                    final CredentialsProvider credentialsProvider =
                            new BasicCredentialsProvider();
                    credentialsProvider.setCredentials(AuthScope.ANY,
                            new UsernamePasswordCredentials(repository.userName, repository.password));
                    builder.setHttpClientConfigCallback(httpClientBuilder -> httpClientBuilder
                            .disableAuthCaching()
                            .setSSLHostnameVerifier((s, sslSession) -> repository.ignoreSSL)
                            .setDefaultCredentialsProvider(credentialsProvider));
                }
                repository.elasticClient = new RestHighLevelClient(builder);
            }
            if (repository.indexName == null) {
                repository.indexName = "bingo";
            }
//            TODO?
//            validate(repository);
            return repository;
        }
    }


    private boolean checkIfIndexExists() throws IOException {
        try {
            return this.elasticClient.indices().exists(new GetIndexRequest(this.indexName), RequestOptions.DEFAULT);
        } catch (IOException e) {
//            TODO logging
            throw e;
        }
    }

    private boolean createIndex(T t) throws IOException {
        CreateIndexRequest request = new CreateIndexRequest(this.indexName);
//        TODO how to think about number of shards?
        request.settings(Settings.builder()
                        .put("index.number_of_shards", 3)
                        .put("index.number_of_replicas", 1)
//                .put("refresh_interval", "5m")
        );

// analyse fields to create proper index
//        Field[] fields = t.getClass().getFields();
//        for (Field f : fields) {
//            String fieldName = f.getName();
//
//        }
        XContentBuilder builder = XContentFactory.jsonBuilder();
        builder.startObject();
        {
            builder.startObject("properties");
            {
                builder.startObject("fingerprint");
                {
                    builder.field("type", "keyword");
                    builder.field("similarity", "boolean");
                }
                builder.endObject();
                builder.startObject("fingerprint_len");
                {
                    builder.field("type", "short");
                }
                builder.endObject();
                builder.startObject("cmf");
                {
                    builder.field("type", "binary");
                }
                builder.endObject();
            }
            builder.endObject();
        }
        builder.endObject();
        request.mapping(builder);

        try {
            CreateIndexResponse createIndexResponse = this.elasticClient.indices().create(request, RequestOptions.DEFAULT);
        } catch (IOException e) {
//            TODO logging
            throw e;
        }
        return false;
    }


    @Override
    public Stream<T> stream() {
        return new ElasticStream<>(this.elasticClient, this.indexName);
    }

    @Override
    public boolean indexRecords(List<T> records) throws IOException {
        if (!checkIfIndexExists())
            createIndex(records.get(0));
        BulkRequest request = new BulkRequest();
        for (T t : records) {
            XContentBuilder builder = XContentFactory.jsonBuilder();
            builder.startObject();
            {
//                todo need to iterate over fields and add content where exists
                builder.array("fingerprint", t.getFingerprint());
                builder.field("cml", t.getCml());
                for (Map.Entry<String, Object> e : t.getObjects().entrySet()) {
                    builder.field(e.getKey(), e.getValue());
                }
            }
            builder.endObject();
            request.add(new IndexRequest(this.indexName)
                    .source(builder));
        }
        this.elasticClient.bulkAsync(request, RequestOptions.DEFAULT, new ActionListener<BulkResponse>() {
            @Override
            public void onResponse(BulkResponse bulkItemResponses) {
                System.out.println("Bulk indexed completed");
            }

            @Override
            public void onFailure(Exception e) {
                System.out.println(e);
            }
        });
        return true;
    }
}
