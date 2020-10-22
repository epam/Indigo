package com.epam.indigo.model.fields;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord;


public class CustomObject {

    public static void map(IndigoObject indigoObject, IndigoRecord.IndigoRecordBuilder builder) {
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        builder.withCustomObject("name", indigoObject.name());
    }

}
