package com.dc.builder;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.lang.reflect.GenericSignatureFormatError;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import com.dc.builder.annotations.Ignore;
import com.dc.builder.java.JavaBuilder;
import com.dc.indigo.core.Item;

import freemarker.cache.ClassTemplateLoader;
import freemarker.template.Configuration;
import freemarker.template.Template;
import freemarker.template.TemplateException;
import freemarker.template.TemplateExceptionHandler;

public abstract class CodeBuilder {

	public static class ClassItem {
		private String packageName;
		private String[] imports;
		private String interfaceName;
		private String superClassName;
		private List<String> allInterfacesNames;
		private List<MethodItem> methods;
		private Class<?> theClass;

		public String getPackageName() {
			return packageName;
		}

		public String[] getImports() {
			return imports;
		}

		public String getInterfaceName() {
			return interfaceName;
		}

		public String getSuperClassName() {
			return superClassName;
		}

		public List<String> getAllInterfacesNames() {
			return allInterfacesNames;
		}

		public List<MethodItem> getMethods() {
			return methods;
		}

		public Class<?> getTheClass() {
			return theClass;
		}
	}

	public static class MethodItem {
		private String type;
		private boolean isRetItem;
		private String name;
		private List<ParamItem> params;
		public String simpleTypeName;

		public String getSimpleTypeName() {
			return simpleTypeName;
		}

		public String getType() {
			return type;
		}

		public boolean getIsRetItem() {
			return isRetItem;
		}

		public String getName() {
			return name;
		}

		public List<ParamItem> getParams() {
			return params;
		}
	}

	public static class ParamItem {

		private String type;
		private String name;
		private boolean isItem;

		public String getType() {
			return type;
		}

		public String getName() {
			return name;
		}

		public boolean getIsItem() {
			return isItem;
		}
	}

	public static Configuration cfg = new Configuration(Configuration.VERSION_2_3_30);

	public static void main(String[] args) {

		// Recommended settings for new projects:
		cfg.setDefaultEncoding("UTF-8");
		cfg.setTemplateExceptionHandler(TemplateExceptionHandler.RETHROW_HANDLER);
		cfg.setLogTemplateExceptions(false);
		cfg.setWrapUncheckedExceptions(true);
		cfg.setFallbackOnNullLoopVariable(false);
		cfg.setTemplateLoader(new ClassTemplateLoader(CodeBuilder.class, "templates"));

		CodeBuilder[] builders = { new JavaBuilder() };

		try {
			List<ClassItem> classes = Arrays.asList(getClasses("com.dc.indigo")).stream()
					.filter(e -> e.isInterface() && Item.class.isAssignableFrom(e) && Item.class != e && e.getAnnotation(Ignore.class) == null).map(e -> {
						ClassItem classItem = new ClassItem();
						classItem.theClass = e.getClass();
						classItem.packageName = e.getPackageName();
						classItem.imports = new String[0];
						classItem.interfaceName = e.getSimpleName();

						Class<?> superclass = e.getInterfaces()[0];
						classItem.superClassName = superclass.getPackage().getName() + ".impl."
								+ superclass.getSimpleName() + (superclass == Item.class ? "" : "Generated") + "Impl";
						classItem.allInterfacesNames = Arrays.asList(e.getInterfaces()).stream().map(ee -> ee.getName())
								.collect(Collectors.toList());
						classItem.methods = Arrays.asList(e.getDeclaredMethods()).stream()
								.filter(m -> (m.getModifiers() & Modifier.PUBLIC) != 0 && !m.isDefault()).map(m -> {
									MethodItem methodItem = new MethodItem();

									methodItem.type = m.getReturnType().getName();
									methodItem.simpleTypeName = m.getReturnType().getSimpleName();

									try {
										String params = String.join(", ",
												Arrays.asList(((ParameterizedType) m.getGenericReturnType())
														.getActualTypeArguments()).stream().map(ee -> ee.getTypeName())
														.collect(Collectors.toList()));
										methodItem.type += "<" + params + ">";
									} catch (ClassCastException | GenericSignatureFormatError ex) {
									}

									methodItem.isRetItem = Item.class.isAssignableFrom(m.getReturnType());
									methodItem.name = m.getName();
									methodItem.params = Arrays.asList(m.getParameters()).stream().map(p -> {
										ParamItem paramItem = new ParamItem();
										paramItem.type = p.getType().getName();
										paramItem.name = p.getName();
										paramItem.isItem = Item.class.isAssignableFrom(p.getType());
										return paramItem;
									}).collect(Collectors.toList());
									return methodItem;
								}).collect(Collectors.toList());
						return classItem;
					}).collect(Collectors.toList());

			classes.forEach(c -> {
				System.out.println(c.getInterfaceName());
				Arrays.asList(builders).stream().forEach(e -> e.buildImpl(c));
			});

		} catch (ClassNotFoundException |

				IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	abstract public void buildImpl(ClassItem classItem);

	protected void applyTemplate(Object item, String templateName, String outFilePath, boolean overwrite) {
		try (Writer out = createFile(outFilePath, overwrite)) {
			if (out != null) {
				Template temp = CodeBuilder.cfg.getTemplate(templateName);
				Map<String, Object> m = new HashMap<>();
				m.put("Item", item);
				temp.process(m, out);
			}
		} catch (TemplateException | IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	protected Writer createFile(String pathname, boolean overwrite) {
		Writer out = null;
		File file = new File(pathname);
		if (overwrite || !file.exists()) {
			file.getParentFile().mkdirs();
			try {
				out = new OutputStreamWriter(new FileOutputStream(file));
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		return out;
	}

	/**
	 * Scans all classes accessible from the context class loader which belong to
	 * the given package and subpackages.
	 *
	 * @param packageName The base package
	 * @return The classes
	 * @throws ClassNotFoundException
	 * @throws IOException
	 */
	public static Class<?>[] getClasses(String packageName) throws ClassNotFoundException, IOException {
		ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
		assert classLoader != null;
		String path = packageName.replace('.', '/');
		Enumeration<URL> resources = classLoader.getResources(path);
		List<File> dirs = new ArrayList<File>();
		while (resources.hasMoreElements()) {
			URL resource = resources.nextElement();
			dirs.add(new File(resource.getFile()));
		}
		ArrayList<Class<?>> classes = new ArrayList<>();
		for (File directory : dirs) {
			classes.addAll(findClasses(directory, packageName));
		}
		return classes.toArray(new Class[classes.size()]);
	}

	/**
	 * Recursive method used to find all classes in a given directory and subdirs.
	 *
	 * @param directory   The base directory
	 * @param packageName The package name for classes found inside the base
	 *                    directory
	 * @return The classes
	 * @throws ClassNotFoundException
	 */
	private static List<Class<?>> findClasses(File directory, String packageName) throws ClassNotFoundException {
		List<Class<?>> classes = new ArrayList<>();
		if (!directory.exists()) {
			return classes;
		}
		File[] files = directory.listFiles();
		for (File file : files) {
			if (file.isDirectory()) {
				assert !file.getName().contains(".");
				classes.addAll(findClasses(file, packageName + (packageName.isEmpty() ? "" : ".") + file.getName()));
			} else if (file.getName().endsWith(".class")) {
				classes.add(Class.forName(packageName + (packageName.isEmpty() ? "" : ".")
						+ file.getName().substring(0, file.getName().length() - 6)));
			}
		}
		return classes;
	}
}
