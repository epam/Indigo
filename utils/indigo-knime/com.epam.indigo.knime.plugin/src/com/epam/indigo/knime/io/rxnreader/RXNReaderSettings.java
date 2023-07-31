package com.epam.indigo.knime.io.rxnreader;

import org.knime.core.node.InvalidSettingsException;
import org.knime.core.node.NodeSettingsRO;
import org.knime.core.node.NodeSettingsWO;

public class RXNReaderSettings {

	public String fileName;
	

	public void loadSettings(final NodeSettingsRO settings) throws InvalidSettingsException {
		fileName = settings.getString("fileName");
	}

	public void loadSettingsForDialog(final NodeSettingsRO settings) {
	    fileName = settings.getString("fileName", null);
	}

	public void saveSettings(final NodeSettingsWO settings) {
		if (fileName != null)
			settings.addString("fileName", fileName);
	}
}
