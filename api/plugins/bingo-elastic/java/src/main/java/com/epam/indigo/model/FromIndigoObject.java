package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.IndigoObject;

public class FromIndigoObject {

    public static IndigoRecord build(IndigoObject indigoObject) throws BingoElasticException {
        return build(indigoObject, error -> {});
    }

    public static IndigoRecord build(IndigoObject indigoObject, ErrorHandler errorHandler) throws BingoElasticException {
        indigoObject.aromatize();
        IndigoRecordMolecule.IndigoRecordBuilder builder = new IndigoRecordMolecule.IndigoRecordBuilder();
        builder.withIndigoObject(indigoObject);
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        builder.withErrorHandler(errorHandler);
        return builder.build();
    }

}
