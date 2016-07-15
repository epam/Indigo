mvn install
mvn -pl chem-query-plugin package assembly:single
cp chem-query-plugin/target/chem-query-plugin-1.0-SNAPSHOT-jar-with-dependencies.jar dist/chem-query-plugin.jar
cd docker
sudo docker-compose rm indigo-lucene-dev
sudo docker-compose up indigo-lucene-dev
