package com.epam.indigo.elastic;

import com.epam.indigo.GenericRepository;
import com.epam.indigo.model.IndigoRecord;
import org.apache.http.HttpHost;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.admin.indices.delete.DeleteIndexRequest;
import org.elasticsearch.action.bulk.BulkRequest;
import org.elasticsearch.action.bulk.BulkResponse;
import org.elasticsearch.action.index.IndexRequest;
import org.elasticsearch.action.support.master.AcknowledgedResponse;
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

/**
 * Class responsible for all operations with Elasticsearch
 * Have ability to index, delete, produce stream for further operations like similarity match, filtering on extra textual fields, etc
 *
 * @param <T>
 */
public class ElasticRepository<T extends IndigoRecord> implements GenericRepository<T> {

    private String indexName = "bingo";
    private String hostName;
    private int port;
    private String scheme;
    private String userName;
    private String password;
    private RestHighLevelClient elasticClient;
    private boolean ignoreSSL;
    private int numShards = 1;
    private int numReplicas = 1;
    private String refreshInterval = "5m";

    private ElasticRepository() {
    }

    public static class ElasticRepositoryBuilder<T extends IndigoRecord> {
        private final List<Consumer<ElasticRepository<T>>> operations;

        public ElasticRepositoryBuilder() {
            this.operations = new ArrayList<>();
        }

        public ElasticRepositoryBuilder<T> withIndexName(String indexName) {
            operations.add(repo -> repo.indexName = indexName);
            return this;
        }

        public ElasticRepositoryBuilder<T> withHostName(String hostName) {
            operations.add(repo -> repo.hostName = hostName);
            return this;
        }

        public ElasticRepositoryBuilder<T> withPort(Integer port) {
            operations.add(repo -> repo.port = port);
            return this;
        }

        public ElasticRepositoryBuilder<T> withScheme(String scheme) {
            operations.add(repo -> repo.scheme = scheme);
            return this;
        }

        public ElasticRepositoryBuilder<T> withUserName(String userName) {
            operations.add(repo -> repo.userName = userName);
            return this;
        }

        public ElasticRepositoryBuilder<T> withPassword(String password) {
            operations.add(repo -> repo.password = password);
            return this;
        }

        public ElasticRepositoryBuilder<T> withRestClient(RestHighLevelClient restHighLevelClient) {
            operations.add(repo -> repo.elasticClient = restHighLevelClient);
            return this;
        }

        public ElasticRepositoryBuilder<T> withIgnoreSSL(boolean ignoreSSL) {
            operations.add(repo -> repo.ignoreSSL = ignoreSSL);
            return this;
        }

        public ElasticRepositoryBuilder<T> withShards(int numShards) {
            operations.add(repo -> repo.numShards = numShards);
            return this;
        }

        public ElasticRepositoryBuilder<T> withReplicas(int numReplicas) {
            operations.add(repo -> repo.numReplicas = numReplicas);
            return this;
        }

        public ElasticRepositoryBuilder<T> withRefreshInterval(String refreshInterval) {
            operations.add(repo -> repo.refreshInterval = refreshInterval);
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
        request.settings(Settings.builder()
                .put("index.number_of_shards", this.numShards)
                .put("index.number_of_replicas", this.numReplicas)
                .put("refresh_interval", this.refreshInterval)
        );
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
                    builder.field("type", "integer");
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
            return createIndexResponse.isAcknowledged();
        } catch (IOException e) {
//            TODO logging
            throw e;
        }
    }


    @Override
    public Stream<T> stream() {
        return new ElasticStream<>(this.elasticClient, this.indexName);
    }

    public boolean indexRecord(T record) throws IOException {
        List<T> rec = new ArrayList<>();
        rec.add(record);
        return indexRecords(rec);
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
                builder.field("fingerprint_len", t.getFingerprint().size());
                builder.field("cmf", t.getCmf());
                for (Map.Entry<String, Object> e : t.getObjects().entrySet()) {
                    // todo: allow extend by users?
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


    @Override
    public boolean deleteAllRecords() throws IOException {
        DeleteIndexRequest request = new DeleteIndexRequest(this.indexName);
        try {
            AcknowledgedResponse delete = this.elasticClient.indices().delete(request, RequestOptions.DEFAULT);
            return delete.isAcknowledged();
        } catch (IOException e) {
//            TODO logging
            throw e;
        }
    }
}
