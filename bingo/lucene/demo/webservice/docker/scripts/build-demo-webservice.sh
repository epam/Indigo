cd src
mvn clean
mvn install -P docker-compose
cp demo/webservice/target/webservice-1.0-SNAPSHOT.war /dist/webservice.war
