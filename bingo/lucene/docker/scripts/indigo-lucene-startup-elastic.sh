#!/usr/bin/env bash
if [[ "$WAIT_FOR_CHEMQUERY_BUILD" == true ]]; then
    echo "Waiting for chem-query plugin to be built..."
    inotifywait -e attrib /build-status/chem-query && echo "Seems that build is finished."
fi

#ln -s /opt/solr/plugin/chem-query-plugin.jar /opt/solr/server/solr/moldocs/lib/chem-query-plugin.jar
#ln -s /opt/solr/conf/ /opt/solr/server/solr/moldocs/

echo "Starting elastic..."
#/opt/elasticsearch/bin/elasticsearch
#/usr/share/elasticsearch/bin/elasticsearch
elasticsearch

echo "Waiting..."
sleep 10s

#curl "http://localhost:8983/solr/admin/cores?action=CREATE&name=moldocs&instanceDir=moldocs&config=solrconfig.xml&schema=schema.xml&dataDir=data"

${WAIT_FOR_CHEMQUERY_BUILD=false}

while true; do
    sleep 1000
done
