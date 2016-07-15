package com.epam.indigolucene.service.indexer;

import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.service.exceptions.UnknownFileFormatException;
import com.epam.indigolucene.service.generated.TestSchema;
import com.epam.indigolucene.service.util.FilesUtil;
import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Autowired;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Created by Artem Malykh on 14.07.16.
 */
public class Indexer {
    private static Logger logger = Logger.getLogger(Indexer.class);

    public static void indexChemData(CollectionRepresentation<TestSchema> collection, String chemDataFolder) {
        if (chemDataFolder == null || chemDataFolder.isEmpty()) {
            logger.info("No data to index, exiting indexer");
            return;
        }
        try {
            Files.walk(Paths.get(chemDataFolder))
                    .filter(Files::isRegularFile)
                    .forEach(p -> {
                        String filePath = chemDataFolder + "/" + p.getFileName();
                        logger.info("Trying to index " + filePath  + "...");
                        boolean ok = true;
                        try {
                            FilesUtil.indexFile(filePath, collection);
                        } catch (UnknownFileFormatException e) {
                            logger.error(e);
                            ok = false;
                        }
                        if (ok) {
                            logger.info("successfully done.");
                        }
                    });
        } catch (IOException e) {
            logger.error("Error while indexing files.", e);
        }
    }
}
