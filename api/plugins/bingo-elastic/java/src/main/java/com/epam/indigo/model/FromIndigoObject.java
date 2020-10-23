package com.epam.indigo.model;

import com.epam.indigo.BingoElasticException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.fields.CustomObject;

public class FromIndigoObject {

    public static IndigoRecord build(IndigoObject indigoObject) throws BingoElasticException {

        IndigoRecord.IndigoRecordBuilder builder = new IndigoRecord.IndigoRecordBuilder().withIndigoObject(
                indigoObject
        );
        CustomObject.map(indigoObject, builder);
        return builder.build();
    }

}
