package com.dc.builder.java;

import com.dc.builder.CodeBuilder;

public class JavaBuilder extends CodeBuilder {

	class ClassDescription {
	}

	@Override
	public void buildImpl(ClassItem classItem) {

		applyTemplate(classItem, "java/JavaImpl.ftl", "src/main/java/" + classItem.getPackageName().replace('.', '/') + "/impl/"
				+ classItem.getInterfaceName() + "GeneratedImpl.java", true);
	}
}
