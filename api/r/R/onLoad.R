.onLoad <- function(libname, pkgname) {
	basedir <- system.file(package="rindigo")

   packageStartupMessage('Loading rindigo')
   packageStartupMessage('For a complete list of package functions, use ls("package:rindigo")')
}