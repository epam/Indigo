set -e
cd /indigo-lucene

#TODO: remove skipping tests
mvn clean install
cd chem-query-plugin
mvn clean compile package assembly:single
cp target/chem-query-plugin-1.0-SNAPSHOT-jar-with-dependencies.jar /dist/chem-query-plugin.jar