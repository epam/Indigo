package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.IndigoObject;

/**
 * Class for building IndigoRecordMolecule and IndigoRecordReaction from IndigoObject
 * Usually this class is used for converting IndigoObject before saving into Elastic.
 */

public class FromIndigoObject {

    ////////////////////////////////////////////////////////////////
    //
    // MOLECULE
    //
    ////////////////////////////////////////////////////////////////

    /**
     * Build IndigoRecordMolecule from IndigoObject with empty ErrorHandler. All errors on building will be ignored
     *
     * @param indigoObject
     * @return
     * @throws BingoElasticException
     */
    public static IndigoRecordMolecule buildMolecule(IndigoObject indigoObject) throws BingoElasticException {
        return buildMolecule(indigoObject, error -> {});
    }

    /**
     * Build IndigoRecordMolecule from IndigoObject. ErrorHandler called on building errors
     *
     * @param indigoObject
     * @param errorHandler
     * @return
     * @throws BingoElasticException
     */
    public static IndigoRecordMolecule buildMolecule(IndigoObject indigoObject, ErrorHandler errorHandler) throws BingoElasticException {
        indigoObject.aromatize();
        IndigoRecordMolecule.IndigoRecordBuilder builder = new IndigoRecordMolecule.IndigoRecordBuilder();
        builder.withIndigoObject(indigoObject);
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        builder.withErrorHandler(errorHandler);
        return builder.build();
    }

    ////////////////////////////////////////////////////////////////
    //
    // REACTION
    //
    ////////////////////////////////////////////////////////////////

    /**
     * Build InigoRecordReaction from IndigoObject. All errors on building will be ignored
     * @param indigoObject
     * @return
     * @throws BingoElasticException
     */
    public static IndigoRecordReaction buildReaction(IndigoObject indigoObject) throws BingoElasticException {
        return buildReaction(indigoObject, error -> {});
    }

    /**
     *
     * Build IndigoRecordReaction from IndigoObject. ErrorHandler called on building errors
     *
     * @param indigoObject
     * @param errorHandler
     * @return
     * @throws BingoElasticException
     */
    public static IndigoRecordReaction buildReaction(IndigoObject indigoObject, ErrorHandler errorHandler) throws BingoElasticException {

        IndigoRecordReaction.IndigoRecordBuilder builder = new IndigoRecordReaction.IndigoRecordBuilder();
        builder.withIndigoObject(indigoObject);
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        builder.withErrorHandler(errorHandler);
        return builder.build();
    }

}