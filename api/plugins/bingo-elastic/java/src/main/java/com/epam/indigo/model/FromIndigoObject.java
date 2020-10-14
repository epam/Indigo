package com.epam.indigo.model;

import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.IndigoRecord;

public class FromIndigoObject {

    IndigoObject indigoObject;

    public FromIndigoObject(IndigoObject indigoObject) {
        this.indigoObject = indigoObject;
    }

    public IndigoRecord get() {

        IndigoRecord.IndigoRecordBuilder builder = new IndigoRecord.IndigoRecordBuilder().withIndigoObject(
                indigoObject
        );
        for (IndigoObject prop : indigoObject.iterateProperties()) {
            builder.withCustomObject(prop.name(), prop.rawData());
        }
        return builder.build();
    }

}
