package com.epam.indigolucene.service.controller;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.IndigoHolder;
import com.epam.indigolucene.service.exceptions.UnknownFileFormatException;
import com.epam.indigolucene.service.generated.TestSchema;
import com.epam.indigolucene.service.model.MoleculeData;
import com.epam.indigolucene.service.model.Result;
import com.epam.indigolucene.service.model.SimpleStructureQuery;
import com.epam.indigolucene.service.util.FilesUtil;
import io.swagger.annotations.ApiOperation;
import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * Created by Artem Malykh on 16.06.16.
 */
@RestController
public class SearchController {

    private static final String UPLOAD_DIR = ".";
    private static final Logger logger = Logger.getLogger(SearchController.class);
    @Autowired
    CollectionRepresentation<TestSchema> molCollection;

    @ApiOperation(value="Search molecules by substructure")
    @RequestMapping(value = "/findBySubstructure", method = RequestMethod.POST)
    public final Result<List<MoleculeData>> findSubstructure(@RequestBody final SimpleStructureQuery query) throws Exception {
        List<MoleculeData> res = new LinkedList<>();
        molCollection.find().filter(TestSchema.MOL.unsafeHasSubstructure(query.getStructure()))
                            .limit(query.getLimit())
                            .offset(query.getOffset()).processWith(maps -> {
                                for (Map<String, Object> map : maps) {
                                    IndigoObject io = IndigoHolder.getIndigo().unserialize((byte[]) map.get(TestSchema.MOL.getName()));
                                    res.add(new MoleculeData((String) map.get(TestSchema.MOL_ID.getName()), io.smiles()));
                                }
                            });
        return Result.success(res);
    }

    @ApiOperation(value="Search molecules with exact search")
    @RequestMapping(value = "/findExact", method = RequestMethod.POST)
    public final Result<List<MoleculeData>> findExact(@RequestBody final SimpleStructureQuery query) throws Exception {
        List<MoleculeData> res = new LinkedList<>();
        molCollection.find().filter(TestSchema.MOL.unsafeExactMatches(query.getStructure()))
                .limit(query.getLimit())
                .offset(query.getOffset()).processWith(maps -> {
            for (Map<String, Object> map : maps) {
                IndigoObject io = IndigoHolder.getIndigo().unserialize((byte[]) map.get(TestSchema.MOL.getName()));
                res.add(new MoleculeData((String) map.get(TestSchema.MOL_ID.getName()), io.smiles()));
            }
        });
        return Result.success(res);
    }

    @ApiOperation(value="Search molecules with exact search")
    @RequestMapping(value = "/findSubstructureNotExact", method = RequestMethod.POST)
    public final Result<List<MoleculeData>> findSubstructureNotExact(@RequestBody final SimpleStructureQuery query) throws Exception {
        List<MoleculeData> res = new LinkedList<>();
        molCollection.find().filter(TestSchema.MOL.unsafeExactMatches(query.getStructure()).not().
                                    and(TestSchema.MOL.unsafeHasSubstructure(query.getStructure())))
                .limit(query.getLimit())
                .offset(query.getOffset()).processWith(maps -> {
            for (Map<String, Object> map : maps) {
                IndigoObject io = IndigoHolder.getIndigo().unserialize((byte[]) map.get(TestSchema.MOL.getName()));
                res.add(new MoleculeData((String) map.get(TestSchema.MOL_ID.getName()), io.smiles()));
            }
        });
        return Result.success(res);
    }

    @ApiOperation(value="Upload file and index it")
    @RequestMapping(value="/indexFile", method = RequestMethod.POST)
    public final Result<Integer> indexFile(@RequestParam("file") MultipartFile file) throws UnknownFileFormatException {
        String fileName = file.getOriginalFilename() + UUID.randomUUID();
        Path path = Paths.get(UPLOAD_DIR, fileName);
        if (!file.isEmpty()) {
            try {
                Files.copy(file.getInputStream(), path);
            } catch (IOException |RuntimeException e) {
                logger.error(e);
                return Result.error("Could not upload file " + file.getOriginalFilename());
            }
        } else {
            return Result.error("File is empty.");
        }
        return Result.success(FilesUtil.indexFile(path.toString(), molCollection));
    }

    @ApiOperation(value="Just a simple ping")
    @RequestMapping(value = "/ping", method = RequestMethod.POST)
    public final Result<Long> ping() {
        return Result.success(System.currentTimeMillis());
    }


}
