if ["%KNIMEDIR%"] == [] (
	echo "KNIMEDIR is not defined! Error.";
	exit;
) else (
	"%KNIMEDIR%\eclipse" -application org.knime.testing.KNIME_TESTING_APPLICATION -consoleLog -noexit -nosplash -root workspace -pattern ".*"
)


