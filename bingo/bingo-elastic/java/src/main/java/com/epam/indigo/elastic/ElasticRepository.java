package com.epam.indigo.elastic;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.GenericRepository;
import com.epam.indigo.model.IndigoRecord;
import org.apache.http.HttpHost;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.ElasticsearchStatusException;
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

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.io.IOException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Base64;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;
import java.util.stream.Stream;

import static com.epam.indigo.model.NamingConstants.*;

/**
 * Class responsible for all operations with Elasticsearch
 * Have ability to index, delete, produce stream for further operations like similarity match, filtering on extra textual fields, etc
 */
public class ElasticRepository<T extends IndigoRecord> implements GenericRepository<T> {

    private String indexName;
    private String hostName;
    private int port;
    private String scheme;
    private String userName;
    private String password;
    private RestHighLevelClient elasticClient;
    private boolean ignoreSSL;
    private int numShards = 1;
    private int numReplicas = 0;
    private String refreshInterval = "5m";

    private ElasticRepository() {
    }

    private boolean checkIfIndexExists() throws BingoElasticException {
        try {
            return this.elasticClient.indices().exists(new GetIndexRequest(this.indexName), RequestOptions.DEFAULT);
        } catch (IOException e) {
            throw new BingoElasticException("Couldn't check if index exists", e.getCause());
        }
    }

    private boolean createIndex() throws IOException {
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
                builder.startObject(SIM_FINGERPRINT);
                {
                    builder.field("type", "keyword");
                    builder.field("similarity", "boolean");
                }
                builder.endObject();
                builder.startObject(SIM_FINGERPRINT_LEN);
                {
                    builder.field("type", "integer");
                }
                builder.endObject();
                builder.startObject(SUB_FINGERPRINT);
                {
                    builder.field("type", "keyword");
                    builder.field("similarity", "boolean");
                }
                builder.endObject();
                builder.startObject(SUB_FINGERPRINT_LEN);
                {
                    builder.field("type", "integer");
                }
                builder.endObject();
                builder.startObject(CMF);
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
            throw new BingoElasticException("Couldn't create index in Elasticsearch", e.getCause());
        }
    }

    @Override
    public Stream<T> stream() {
        return new ElasticStream<>(this.elasticClient, this.indexName);
    }

    void indexRecord(T record) throws IOException {
        indexRecords(Collections.singletonList(record), 1);
    }

    @Override
    public void indexRecords(Iterable<T> records, int batchSize) throws IOException {
        indexRecords(records, batchSize, new ActionListener<BulkResponse>() {
            @Override
            public void onResponse(BulkResponse bulkResponse) {
//                do nothing
            }

            @Override
            public void onFailure(Exception e) {
//              do nothing
            }
        });
    }

    public Iterable<List<T>> splitToBatches(Iterable<T> records, int batchSize) {

        Iterator<T> instRecords = records.iterator();
        return () -> new Iterator<List<T>>() {

            @Override
            public boolean hasNext() {
                return instRecords.hasNext();
            }

            @Override
            public List<T> next() {
                List<T> acc = new ArrayList<>();
                while (instRecords.hasNext() && acc.size() < batchSize) {
                    acc.add(instRecords.next());
                }
                return acc;
            }
        };
    }

    @Override
    public void indexRecords(Iterable<T> flatRecords, int batchSize, ActionListener<BulkResponse> actionListener) throws IOException {
        if (!checkIfIndexExists())
            createIndex();

        for (List<T> records : splitToBatches(flatRecords, batchSize)) {
            for (T t : records) {
                BulkRequest request = new BulkRequest();
                XContentBuilder builder = XContentFactory.jsonBuilder();
                builder.startObject();
                {
                    builder.array(SIM_FINGERPRINT, t.getSimFingerprint());
                    builder.field(SIM_FINGERPRINT_LEN, t.getSimFingerprint().size());
                    builder.array(SUB_FINGERPRINT, t.getSubFingerprint());
                    builder.field(SUB_FINGERPRINT_LEN, t.getSubFingerprint().size());
                    builder.field(CMF, Base64.getEncoder().encodeToString(t.getCmf()));
                    builder.field(NAME, t.getName());
                    for (Map.Entry<String, Object> e : t.getObjects().entrySet()) {
                        // todo: allow extend by users?
                        builder.field(e.getKey(), e.getValue());
                    }
                }
                builder.endObject();
                request.add(new IndexRequest(this.indexName)
                        .source(builder));
                this.elasticClient.bulkAsync(request, RequestOptions.DEFAULT, actionListener);
            }
        }
//        TODO do we need it?
//        FlushRequest flushRequest = new FlushRequest();
//        this.elasticClient.indices().flushAsync(flushRequest, RequestOptions.DEFAULT);
//        ForceMergeRequest forceMergeRequest = new ForceMergeRequest();
//        this.elasticClient.indices().forcemerge(forceMergeRequest, RequestOptions.DEFAULT);
    }

    @Override
    public boolean deleteAllRecords() {
        DeleteIndexRequest request = new DeleteIndexRequest(this.indexName);
        try {
            AcknowledgedResponse delete = this.elasticClient.indices().delete(request, RequestOptions.DEFAULT);
            return delete.isAcknowledged();
        } catch (IOException e) {
            throw new BingoElasticException("Couldn't delete records in Elasticsearch", e.getCause());
        } catch (ElasticsearchStatusException e) {
            if (e.status().name().equals("NOT_FOUND")) {
                // skip error on empty index
                return true;
            }
            throw new BingoElasticException("Couldn't delete records in Elasticsearch", e.getCause());
        }
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
                            .setSSLContext(initSSLContext())
                            .setSSLHostnameVerifier((s, sslSession) -> repository.ignoreSSL)
                            .setDefaultCredentialsProvider(credentialsProvider));
                }
                repository.elasticClient = new RestHighLevelClient(builder);
            }
            validate(repository);
            return repository;
        }

        private SSLContext initSSLContext() {
            SSLContext sc = null;
            try {
                TrustManager[] trustAllCerts = new TrustManager[]{new X509TrustManager() {
                    public X509Certificate[] getAcceptedIssuers() {
                        return null;
                    }

                    public void checkClientTrusted(X509Certificate[] certs, String authType) {
                    }

                    public void checkServerTrusted(X509Certificate[] certs, String authType) {
                    }
                }
                };
                // Install the all-trusting trust manager
                sc = SSLContext.getInstance("SSL");
                sc.init(null, trustAllCerts, new java.security.SecureRandom());
            } catch (NoSuchAlgorithmException e) {
                throw new BingoElasticException(
                        "Elasticsearch isn't started at");
            } catch (KeyManagementException e) {
                throw new RuntimeException(e);
            }
            return sc;
        }

        private void validate(ElasticRepository<T> repository) {
            boolean ping;
            if (null == repository.indexName || repository.indexName.isEmpty()) {
                throw new BingoElasticException("Elasticsearch indexName must be initialized. Use withIndexName method");
            }
            try {
                ping = repository.elasticClient.ping(RequestOptions.DEFAULT);
                if (!ping) {
                    throw new BingoElasticException("Elasticsearch isn't started at " + repository.hostName + ":" + repository.port);
                }
            } catch (IOException e) {
                throw new BingoElasticException("Elasticsearch isn't started at " + repository.hostName + ":" + repository.port);
            } catch (ElasticsearchException e) {
                throw  new BingoElasticException("Elasticsearch isn't started at " + repository.hostName + ":" + repository.port
                + " " + e.getDetailedMessage());
            }

        }
    }
}
