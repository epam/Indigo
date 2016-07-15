package com.epam.indigolucene.service.util;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.common.SolrUploadStream;
import com.epam.indigolucene.service.exceptions.UnknownFileFormatException;
import com.epam.indigolucene.service.generated.TestSchema;
import com.epam.indigolucene.service.generated.TestSchemaDocument;
import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.propertyeditors.UUIDEditor;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.Function;

/**
 * Created by Artem Malykh on 03.07.16.
 */
public class FilesUtil {

    private static Logger logger = Logger.getLogger(FilesUtil.class);

    private static List<BiFunction<Indigo, String, IndigoObject>> fileIterators = new ArrayList<>();

    static {
        fileIterators.add(Indigo::iterateSmilesFile);
        fileIterators.add(Indigo::iterateSDFile);
        fileIterators.add(Indigo::iterateCDXFile);
    }

    /**
     * Tries to infer file format and index it.
`     * @param fileName file name
     * @return number of records indexed
     * @throws UnknownFileFormatException
     */
    public static int indexFile(String fileName, CollectionRepresentation<TestSchema> molCollection) throws UnknownFileFormatException {
        int records = 0;
        for (int i = 0; i < fileIterators.size(); i++) {
            try {
                try (SolrUploadStream<TestSchema> testSchemaSolrUploadStream = molCollection.uploadStream()) {
                    for (IndigoObject indigoObject : fileIterators.get(i).apply(IndigoHolder.getIndigo(), fileName)) {
                        try {
                            TestSchemaDocument emptyDocument = TestSchema.createEmptyDocument();
                            emptyDocument.setMol(indigoObject);
                            emptyDocument.setMolId(UUID.randomUUID().toString());
                            testSchemaSolrUploadStream.addDocument(emptyDocument);
                            records++;
                        } catch (Exception e) {
                            logger.error("Error while indexing molecule");
                        }
                    }
                }
                break;
            } catch (IOException e) {
                logger.error("Error reading file " + fileName);
            } catch (Exception e) {
                logger.error(e);
                //if we have not any more file iterators, we say that we do not know the file format.
                if (i == fileIterators.size() - 1) {
                    throw new UnknownFileFormatException("Could not infer file format of " + fileName);
                }
                continue;
            }
        }
        return records;
    }
}
