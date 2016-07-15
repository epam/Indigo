WAIT_LOOPS=20
ARTIFACT_NAME="chem-query-plugin.jar"
#wait for artifact to be ready
i=0
while [ ! -f /opt/solr/plugin/tmp-ws/${ARTIFACT_NAME} ]
do
    if [ ${i} -gt ${WAIT_LOOPS} ]; then
        echo "$(date) - still not ready, giving up"
        exit 1
    fi
    i=`expr ${i} + 1`
    echo "Waiting for ${ARTIFACT_NAME} to exist (attempt ${i} of ${WAIT_LOOPS})..."
  sleep 15
done

mv /opt/solr/plugin/tmp-ws/chem-query-plugin.jar /opt/solr/server/solr/moldocs/lib/chem-query-plugin.jar
rm -rf /opt/solr/plugin/tmp-ws/
cp /opt/solr/conf/* /opt/solr/server/solr/moldocs/

echo "Starting solr..."

/opt/solr/bin/solr start ${SOLR_STARTUP_PARAMS}

echo "Waiting..."
sleep 10s

#/opt/solr/bin/solr create -c moldocs
curl "http://localhost:8983/solr/admin/cores?action=CREATE&name=moldocs&instanceDir=moldocs&config=solrconfig.xml&schema=schema.xml&dataDir=data"

${WAIT_FOR_CHEMQUERY_BUILD=false}



while true; do
    sleep 1000
done
