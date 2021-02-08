package com.dc.builder.java;

import java.io.File;
import java.io.IOException;

import com.dc.builder.CodeBuilder;

import freemarker.core.ParseException;
import freemarker.template.MalformedTemplateNameException;
import freemarker.template.TemplateException;
import freemarker.template.TemplateNotFoundException;

public class JavaBuilder extends CodeBuilder {

	class ClassDescription {
	}

	public void buildImpl(ClassItem classItem) throws TemplateNotFoundException, MalformedTemplateNameException,
			ParseException, IOException, TemplateException {

		applyTemplate(classItem, "java/JavaImpl.ftl", "src/main/java/" + classItem.getPackageName().replace('.', '/') + "/impl/"
				+ classItem.getInterfaceName() + "GeneratedImpl.java", true);
	}
}
