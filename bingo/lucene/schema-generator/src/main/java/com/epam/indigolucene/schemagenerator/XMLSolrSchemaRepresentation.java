package com.epam.indigolucene.schemagenerator;

import javax.xml.bind.annotation.*;
import java.util.List;
/**
 * This class is a Java class representation of Solr's schema.xml
 * @author Artem Malykh
 * created on 2016-03-17
 */
@XmlRootElement(name = "schema")
public class XMLSolrSchemaRepresentation {
    @XmlElement(name = "field")
    public List<XMLSolrSchemaRepresentation.Field> fields;

    @XmlRootElement(name = "field")
    @XmlAccessorType(XmlAccessType.FIELD)
    public static class Field {
        @XmlAttribute
        String type;

        @XmlAttribute
        String name;

        @XmlAttribute
        Boolean indexed;

        @XmlAttribute
        Boolean multivalued;
    }

    @XmlElement(name = "fieldtype")
    public List<XMLSolrSchemaRepresentation.FieldType> fieldTypes;

    @XmlAccessorType(XmlAccessType.FIELD)
    public static class FieldType {
        @XmlAttribute
        String name;

        @XmlAttribute(name = "class")
        String clazz;

        @XmlAttribute
        Boolean indexed;

        @XmlAttribute
        Boolean multivalued;
    }
}
