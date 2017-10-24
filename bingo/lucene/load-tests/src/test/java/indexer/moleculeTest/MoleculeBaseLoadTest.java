package indexer.moleculeTest;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.common.exceptions.CommitException;
import com.epam.indigolucene.common.exceptions.DocumentAdditionException;
import indexer.data.generated.TestSchema;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;

import java.io.*;
import java.net.URL;

/**
 * Source:      MoleculeBaseLoadTest.java
 * Created:     09.10.2017
 * Project:     parent-project
 * <p>
 * {@code MoleculeBaseLoadTest} Base tests with some useful methods for extended testing.
 *
 * @author Dmitrii Kuznetsov
 */
class MoleculeBaseLoadTest {
    private static final Logger logger = Logger.getLogger(MoleculeBaseLoadTest.class);

    static CollectionRepresentation<TestSchema> testCollection;

    static {
        BasicConfigurator.configure();
        Logger.getRootLogger().setLevel(Level.INFO);
    }

    static void indexSmilesFile(String fileName) throws Exception {
        try (SolrUploadStream<TestSchema> uploadStream = testCollection.uploadStream()) {
            URL url = MoleculeBaseLoadTest.class.getClassLoader().getResource(fileName);
            if (url == null) {
                String errorMessage = "indexSmilesFile() test: file " + fileName + " isn't found";
                logger.error(errorMessage);
                throw new FileNotFoundException(errorMessage);
            }
            try (BufferedReader br = new BufferedReader(new FileReader(url.getFile()))) {
                String smilesLine;
                while ((smilesLine = br.readLine()) != null) {
                    try {
                        IndigoObject indigoObject = IndigoHolder.getIndigo().loadMolecule(smilesLine);
                        uploadStream.addDocument(TestSchema.createEmptyDocument().setMol(indigoObject));
                    } catch (IndigoException e) {
                        logger.error(e.getMessage());
                    }
                }
            }
        }
    }
}
