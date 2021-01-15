
class CodeGenerator {
	public static void main(String[] args) {

		// Collect all declarations like "interface Molecule" from the specified package
		List<Class<?>> declarations = Arrays.asList(getClasses("com.dc.indigo"));

		declarations.stream.forEach(decl -> {
		/*
		Walk through Java interface '*' using reflection and generate the files:
			1. *GeneratedImpl.java - Java implementation
			2. *.py - Python implementation
			3. *GeneratedAPI.cpp - Java and Python native interface methods
			4. *.h - C++ interface == Java and Python interfaces.

		*/
		})

		// Then implement the *.h in *.cpp.
		// These e.g. Molecule.cpp (Molecule::aMethod()) and source declaration Molecule.java (interface Molecule {aMethod() {....}}) 
		//    are the only manually written code and its interface and functionality are fully equivalent in C++, Java, Python etc.
		//	Consistency	between the declared Java interface and its C++ implementation is checked by compiler.
	}

	/**
	 * Scans all classes accessible from the context class loader which belong
	 * to the given package and subpackages.
	 *
	 * @param packageName
	 *            The base package
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
} 