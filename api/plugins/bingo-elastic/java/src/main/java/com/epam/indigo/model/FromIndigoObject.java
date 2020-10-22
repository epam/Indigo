package com.epam.indigo.model;

import com.epam.indigo.IndigoObject;
import com.epam.indigo.model.fields.CustomObject;

public class FromIndigoObject {

    final IndigoObject indigoObject;

    public FromIndigoObject(IndigoObject indigoObject) {
        this.indigoObject = indigoObject;
    }

    public IndigoRecord get() {

        IndigoRecord.IndigoRecordBuilder builder = new IndigoRecord.IndigoRecordBuilder().withIndigoObject(
                indigoObject
        );
        CustomObject.map(indigoObject, builder);
        return builder.build();
    }

}
