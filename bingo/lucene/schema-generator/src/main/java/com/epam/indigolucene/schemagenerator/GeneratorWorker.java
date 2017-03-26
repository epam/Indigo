package com.epam.indigolucene.schemagenerator;

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.CollectionRepresentation;
import com.epam.indigolucene.common.types.DocumentRepresentation;
import com.epam.indigolucene.common.types.fields.MolField;
import com.epam.indigolucene.common.types.fields.ReactField;
import com.epam.indigolucene.common.types.fields.StringField;
import com.epam.indigolucene.common.types.values.MolValue;
import com.epam.indigolucene.common.types.values.ReactValue;
import com.epam.indigolucene.common.types.values.StringValue;
import com.epam.indigolucene.common.types.values.Value;
import com.epam.indigolucene.common.utils.Utils;
import com.sun.codemodel.*;

import javax.annotation.Generated;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.Unmarshaller;
import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamReader;
import javax.xml.stream.util.StreamReaderDelegate;
import javax.xml.transform.stream.StreamSource;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * Created by Artem Malykh on 30.03.16.
 * Encapsulates main logic for generating schema representations and schema documents.
 */
public class GeneratorWorker {

    private static Map<String, TypeMappingInfo> fieldTypeName2FieldType = new HashMap<String, TypeMappingInfo>();
    private static String FIELD_NAMES = "fieldNames";
    private static final String URL = "url";
    private static final String CORE_NAME = "coreName";
    private static final String COLLECTION = "collection";
    private static final String DOC_POSTFIX = "Document";
    private final Map<String, TypeMappingInfo> presentableFields;
    private final String schemaClassName;
    private final String filePath;
    private final String outputFolder;
    private final String packageName;
    private final XMLSolrSchemaRepresentation parsedSolrSchema;
    private final JCodeModel cm;

    private static class ModelSetterInfo{
        Class<? extends Value> valClazz;
        Class paramClazz;

        public ModelSetterInfo(Class<? extends Value> valClazz, Class paramClazz) {
            this.valClazz = valClazz;
            this.paramClazz = paramClazz;
        }
    }

    private static class TypeMappingInfo {

        /**
         * Class to map
         */
        Class fieldClazz;

        Class valueClazz;

        Class valueTypedSourceClazz;

        /**
         * Mapping will be dony by type alias
         */
        boolean byAlias;

        /**
         *
         * @param clazz
         * @param byAlias
         */
        boolean alwaysSearchable;



        boolean multiple;

        public TypeMappingInfo(Class clazz, Class valueClazz, Class valueTypedSourceClazz, boolean byAlias, boolean alwaysSearchable) {
            this.fieldClazz = clazz;
            this.valueClazz = valueClazz;
            this.valueTypedSourceClazz = valueTypedSourceClazz;
            this.byAlias = byAlias;
            this.alwaysSearchable = alwaysSearchable;

        }

        public TypeMappingInfo(TypeMappingInfo source) {
            this.fieldClazz = source.fieldClazz;
            this.valueClazz = source.valueClazz;
            this.valueTypedSourceClazz = source.valueTypedSourceClazz;
            this.byAlias = source.byAlias;
            this.alwaysSearchable = source.alwaysSearchable;
        }

        public boolean isMultiple() {
            return multiple;
        }

        public void setMultiple(boolean multiple) {
            this.multiple = multiple;
        }
    }

    static {
        fieldTypeName2FieldType.put("T_serMol",        new TypeMappingInfo(MolField.class,     MolValue.class,    IndigoObject.class, true, true));
        fieldTypeName2FieldType.put("T_serReact",        new TypeMappingInfo(ReactField.class,     ReactValue.class,    IndigoObject.class, true, true));
        fieldTypeName2FieldType.put("solr.StrField",   new TypeMappingInfo(StringField.class,  StringValue.class, String.class,       true, false));
        fieldTypeName2FieldType.put("solr.TextField",   new TypeMappingInfo(StringField.class,  StringValue.class, String.class,       true, false));
    }

    public GeneratorWorker(String schemaClassName, String schemaFilePath, String outputFolder, String packageName) throws Exception {
        this.schemaClassName = schemaClassName;
        this.filePath = schemaFilePath;
        this.outputFolder = outputFolder;
        this.packageName = packageName;


        JAXBContext jc = JAXBContext.newInstance(XMLSolrSchemaRepresentation.class);
        Unmarshaller unmarshaller = jc.createUnmarshaller();

        XMLInputFactory xif = XMLInputFactory.newFactory();

        XMLStreamReader xsr = xif.createXMLStreamReader(new StreamSource(schemaFilePath));
        XMLStreamReader xsr1 = new MyStreamReaderDelegate(xsr);

        cm = new JCodeModel();
        parsedSolrSchema = (XMLSolrSchemaRepresentation) unmarshaller.unmarshal(xsr1);
        presentableFields = getPresentableFields();
        System.out.println("Field types found in schema: " + parsedSolrSchema.fieldTypes.size());
        System.out.println("Fields found in schema: " + parsedSolrSchema.fields.size());
        System.out.println("Representable fields: " + presentableFields.size());
    }

    public void generateClasses() throws JClassAlreadyExistsException, IOException, ClassNotFoundException {
        JDefinedClass schemaClass = generateSchemaClass();
        generateDocumentRepresentation(schemaClass);
    }

    public JDefinedClass generateSchemaClass() throws JClassAlreadyExistsException, IOException {


        String fullClassName = getFullClassName(schemaClassName, packageName);

        JDefinedClass schemaClass = cm._class(fullClassName);

        schemaClass.annotate(Generated.class).param("value", JavaSolrSchemaRepresentationGenerator.class.getName());

        for (String fieldName : presentableFields.keySet()) {
            TypeMappingInfo fieldTypeMappingInfo = presentableFields.get(fieldName);

            JClass fieldType = cm.ref(fieldTypeMappingInfo.fieldClazz).narrow(schemaClass);

            schemaClass.field(JMod.PUBLIC | JMod.STATIC | JMod.FINAL,
                    fieldType,
                    fromCamelCaseToUderscore(fieldName),
                    initField(fieldName, fieldType, fieldTypeMappingInfo.multiple));
        }

        JClass fieldNamesType         = cm.ref(Set.class).narrow(String.class); // Set<String>
        JClass fieldNamesTypeImplType = cm.ref(HashSet.class); // HashSet<>

        JFieldVar fieldNames = schemaClass.field(JMod.PRIVATE | JMod.STATIC,
                fieldNamesType,
                FIELD_NAMES,
                JExpr._new(fieldNamesTypeImplType));

        for (String fieldName : presentableFields.keySet()) {
            schemaClass.init().add(fieldNames.invoke("add").arg(fieldName));
        }

        JClass  collectionMethodType = cm.ref(CollectionRepresentation.class).narrow(schemaClass);
        JMethod collectionMethod = schemaClass.method(JMod.PUBLIC | JMod.STATIC,
                collectionMethodType,
                COLLECTION);

        JVar collectionUrlParam      = collectionMethod.param(String.class, URL);
        JVar collectionCoreNameParam = collectionMethod.param(String.class, CORE_NAME);

        collectionMethod.body()._return(JExpr._new(cm.ref(CollectionRepresentation.class).narrow(schemaClass)).arg(collectionUrlParam).arg(collectionCoreNameParam).arg(fieldNames));


        JClass documentClass = cm.ref(packageName + "." + getDocumentClassName(schemaClassName));
        JMethod createEmptyDocumentMethod = schemaClass.method(JMod.PUBLIC | JMod.STATIC,
                documentClass,
                "createEmptyDocument");

        createEmptyDocumentMethod.body()._return(JExpr._new(documentClass));

        cm.build(new File(outputFolder), System.out);

        return schemaClass;
    }

    private TypeMappingInfo getBaseMappingClass(XMLSolrSchemaRepresentation.Field fld)  {
        System.out.println("Searching mapping class for field " + fld.name);
        try {
            String solrTypeName = fld.type;
            //if we found class immediately, return it

            if (fieldTypeName2FieldType.containsKey(solrTypeName)) {
                System.out.println("Found " + solrTypeName + " right away.");
                return getMappingClassFinal(fieldTypeName2FieldType.get(solrTypeName),
                                            fld.indexed != null && fld.indexed,
                                            fld.multivalued != null && fld.multivalued);
            }
            System.out.println("Trying to find type for alias " + solrTypeName);
            //else try to find this class between classes that can be de-aliased
            for (String nameToMap : fieldTypeName2FieldType.keySet()) {
                TypeMappingInfo tmi = fieldTypeName2FieldType.get(nameToMap);

                if (!tmi.byAlias || isFingerprint(solrTypeName)) {
                    continue;
                }
                for (XMLSolrSchemaRepresentation.FieldType fieldType : parsedSolrSchema.fieldTypes) {
                    System.out.println(fieldType.clazz);
                    if (fieldType.name.equals(solrTypeName)) {
                        return getMappingClassFinal(fieldTypeName2FieldType.get(fieldType.clazz),
                                                    fieldType.indexed != null && fieldType.indexed,
                                                    fieldType.multivalued != null && fieldType.multivalued);
                    }
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        }

        return null;
    }

    private boolean isFingerprint(String solrTypeName) {
        //TODO: do more proper check
        return solrTypeName.endsWith("fingerprint");
    }

    private TypeMappingInfo getMappingClassFinal(TypeMappingInfo tm, boolean indexed, boolean multiValued) throws ClassNotFoundException {
        if (tm == null) {
            return null;
        }
        Class clazz = tm.fieldClazz;
        String searchablePrefix = (indexed || tm.alwaysSearchable) ? "searchable.Searchable" : "";
        String fullClassName = clazz.getPackage().getName() + "." + searchablePrefix + clazz.getSimpleName();
        TypeMappingInfo res = new TypeMappingInfo(tm);
        res.setMultiple(multiValued);
        res.fieldClazz = Class.forName(fullClassName);

        return res;
    }

    private static String getFullClassName(String className, String packageName) {
        return packageName + "." + className;
    }

    //TODO: return list of objects already containing classes fields are mapped to and names
    private Map<String, TypeMappingInfo> getPresentableFields() {
        return  parsedSolrSchema.fields.stream().filter(f -> getBaseMappingClass(f) != null).collect(Collectors.toMap(
                field -> field.name,
                (Function<XMLSolrSchemaRepresentation.Field, TypeMappingInfo>) field -> getBaseMappingClass(field)
        ));
    }

    private static JExpression initField(String fieldName, JClass clazz, boolean multiple) {
        return JExpr._new(clazz).arg(fieldName).arg(multiple ? JExpr.TRUE : JExpr.FALSE);
    }

    public static String fromCamelCaseToUderscore(String ccString) {
        String regex = "([a-z])([A-Z]+)";
        String replacement = "$1_$2";
        return ccString.replaceAll(regex, replacement).toUpperCase();
    }

    public static String getDocumentClassName(String schemaClassName) {
        return schemaClassName + DOC_POSTFIX;
    }

    public void generateDocumentRepresentation(JClass schemaClass) throws JClassAlreadyExistsException, IOException, ClassNotFoundException {
        JCodeModel cm = new JCodeModel();
        JDefinedClass docClass = cm._class(getFullClassName(getDocumentClassName(schemaClassName), packageName));


        docClass._implements(cm.ref(DocumentRepresentation.class).narrow(schemaClass));

        docClass.constructor(JMod.PUBLIC);

        JClass fieldsMapType = cm.ref(Map.class).narrow(String.class).narrow(Object.class);

        JMethod fieldsMapMethod = docClass.method(JMod.PUBLIC,
                fieldsMapType,
                "fieldsMap");

        fieldsMapMethod.annotate(Override.class);

        JVar res = fieldsMapMethod.body().decl(JMod.NONE,
                fieldsMapType,
                "res",
                JExpr._new(cm.ref(HashMap.class)));

        for (String fieldName : presentableFields.keySet()) {
            TypeMappingInfo fieldMappingInfo = presentableFields.get(fieldName);


            JClass valueRepresentationClass = cm.ref(fieldMappingInfo.valueClazz).narrow(schemaClass);
            JClass valueSorceClass          = cm.ref(fieldMappingInfo.valueTypedSourceClazz);
            if (fieldMappingInfo.isMultiple()) {
                valueSorceClass = cm.ref(List.class).narrow(valueSorceClass);
            }
            JFieldRef schemaField           = schemaClass.staticRef(fromCamelCaseToUderscore(fieldName));

            JFieldVar fld = docClass.field(JMod.PRIVATE, valueRepresentationClass, fieldName);


            JMethod setter = docClass.method(JMod.PUBLIC, docClass, getFieldSetterName(fieldName));

            JVar setterParam = setter.param(valueSorceClass, "val");

            setter.body().assign(JExpr._this().ref(fld), schemaField.invoke("createValue").arg(setterParam));
            setter.body()._return(JExpr._this());

            fieldsMapMethod.body().add(cm.ref(Utils.class).staticInvoke("addValueToFieldsMap").arg(fld).arg(res));
        }

        fieldsMapMethod.body()._return(res);


        cm.build(new File(outputFolder), System.out);
    }

    private String getFieldSetterName(String fieldName) {
        return "set" + fieldName.substring(0, 1).toUpperCase() + fieldName.substring(1);
    }

    private class MyStreamReaderDelegate extends StreamReaderDelegate {
        public MyStreamReaderDelegate(XMLStreamReader xsr) {
            super(xsr);
        }

        @Override
        public String getAttributeLocalName(int index) {
            String res = super.getAttributeLocalName(index).toLowerCase();
            System.out.println("Attribute: " + res);
            return res;
        }

        @Override
        public String getLocalName() {
            String res = super.getLocalName().toLowerCase();
            System.out.println("local name: " + res);
            return res;
        }
    }
}
