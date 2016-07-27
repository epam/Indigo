package com.epam.indigolucene.schemagenerator;

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
