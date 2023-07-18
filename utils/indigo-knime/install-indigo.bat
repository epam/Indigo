@echo off
set INDIGO_VERSION=1.3.0beta.r16
set INDIGO_LIB_DIR=com.epam.indigo.knime.plugin/lib

for %%a in (indigo indigo-inchi indigo-renderer) do (
	call mvn dependency:get -Dartifact=com.epam.indigo:%%a:%INDIGO_VERSION%:jar -Ddest=%INDIGO_LIB_DIR%/%%a.jar
)

call mvn dependency:get -Dartifact=net.java.dev.jna:jna:3.5.0:jar -Ddest=%INDIGO_LIB_DIR%/jna.jar 