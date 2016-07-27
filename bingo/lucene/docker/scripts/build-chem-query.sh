ARTIFACT_NAME="chem-query-plugin.jar"
WAIT_LOOPS=10
DIST_FOLDER="/dist"
if [ ! -z "$1" ];
then
    DIST_FOLDER=$1
fi

echo "checking if destination folder exists..."
if [ ! -f DIST_FOLDER ];
then
    mkdir ${DIST_FOLDER}
    echo "didn't exist, created"
else
    echo "already exists"
fi

cd /indigo-lucene
mvn install
cd chem-query-plugin
mvn clean compile package assembly:single
cp target/chem-query-plugin-1.0-SNAPSHOT-jar-with-dependencies.jar ${DIST_FOLDER}/${ARTIFACT_NAME}