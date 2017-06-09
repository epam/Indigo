package com.epam.indigolucene.schemagenerator;
/**
 * JavaSolrSchemaRepresentationGenerator is a class, which invokes main mechanisms of Solr's schema.xml parsing and
 * representation of it in Java classes. Example of that kind of representation can be found in "generated" folder.
 *
 * @author Artem Malykh
 */
public class JavaSolrSchemaRepresentationGenerator {
    public static void main(String[] args) throws Exception {
        String schemaClassName = args[0];
        String schemaFilePath  = args[1];
        String outputFolder    = args[2];
        String packageName     = args[3];

        GeneratorWorker gw = new GeneratorWorker(schemaClassName, schemaFilePath, outputFolder, packageName);
        gw.generateClasses();
    }
}
