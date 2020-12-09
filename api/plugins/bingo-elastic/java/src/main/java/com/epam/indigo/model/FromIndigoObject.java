package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.IndigoObject;

public class FromIndigoObject {

    public static IndigoRecord build(IndigoObject indigoObject) throws BingoElasticException {
        indigoObject.aromatize();
        IndigoRecord.IndigoRecordBuilder builder = new IndigoRecord.IndigoRecordBuilder().withIndigoObject(
                indigoObject
        );
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        return builder.build();
    }

}
