ARTIFACT_NAME="webservice.war"
WAIT_LOOPS=10

#wait for artifact to be ready
i=0
while [ ! -f /dist/${ARTIFACT_NAME} ]
do
    if [ ${i} -gt ${WAIT_LOOPS} ]; then
        echo "$(date) - still not ready, giving up"
        exit 1
    fi
    i=`expr ${i} + 1`
    echo "Waiting for ${ARTIFACT_NAME} to exist (attempt ${i} of ${WAIT_LOOPS})..."
  sleep 15
done

SOLR_AVAILABILITY_LOOPS_COUNT=30
SOLR_AVAILABILITY_DELAY=15

i=0
while [ ! `curl -I http://solr-demo:8983/solr/moldocs/query | grep 200 | wc -l` -eq "1" ]
do
  if [ ${i} -gt ${SOLR_AVAILABILITY_LOOPS_COUNT} ]; then
    echo "$(date) - still not ready, giving up"
    exit 1
  fi
  i=`expr ${i} + 1`
  echo "Waiting for solr availability (attempt ${i} of ${SOLR_AVAILABILITY_LOOPS_COUNT})..."
  sleep ${SOLR_AVAILABILITY_DELAY}
done
echo "Solr is available. Continuing"

mv /dist/${ARTIFACT_NAME} ${CATALINA_HOME}/webapps/
./bin/catalina.sh run
