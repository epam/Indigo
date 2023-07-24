/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems, Inc.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 ***************************************************************************/

package com.epam.indigo.knime.plugin;

import static com.epam.indigo.knime.plugin.IndigoPreferenceInitializer.*;

import java.util.StringJoiner;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.Stream;

import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.util.IPropertyChangeListener;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.osgi.framework.BundleContext;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoInchi;

public class IndigoPlugin extends AbstractUIPlugin {
	private static IndigoPlugin plugin;

	public static final String PLUGIN_ID = "com.epam.indigo.knime";

	private static final ReentrantLock lock = new ReentrantLock();

	private static Indigo indigo = new Indigo();
	private static IndigoInchi indigoInchi;

	public IndigoPlugin() {
		super();
		indigo.setOption("serialize-preserve-ordering", true);
		plugin = this;
	}

	public static Indigo getIndigo() {
		return indigo;
	}

	public static IndigoInchi getIndigoInchi() {
		if (indigoInchi == null)
			indigoInchi = new IndigoInchi(indigo);
		return indigoInchi;
	}

	public static void lock() {
		lock.lock();
	}

	public static void unlock() {
		lock.unlock();
	}

	private static int bondLength;
	private static boolean showImplicitHydrogens;
	private static boolean coloring;
	private static boolean valencesVisible;
	private static int molImageWidth;
	private static int molImageHeight;
	private static boolean enableRendering;
	private static String labelMode;
	private static String superatomMode;
	private static String backgroundColor;
	private static String baseColor;

	private String normalizeColor(String color) {
		StringJoiner normaliazed = new StringJoiner(",");
		Stream.of(color.split(",")).
					map(Double::valueOf).
					map(d -> d / 255.0).
					forEach(d -> normaliazed.add(d.toString()));
		return normaliazed.toString();
	}

	@Override
	public void start(final BundleContext context) throws Exception {
		super.start(context);
		final IPreferenceStore pStore = getDefault().getPreferenceStore();

		pStore.addPropertyChangeListener(new IPropertyChangeListener() {
			public void propertyChange(final PropertyChangeEvent event) {
				if (PREF_BOND_LENGTH.equals(event.getProperty()))
					bondLength = pStore.getInt(PREF_BOND_LENGTH);
				else if (PREF_SHOW_IMPLICIT_HYDROGENS.equals(event.getProperty()))
					showImplicitHydrogens = pStore.getBoolean(PREF_SHOW_IMPLICIT_HYDROGENS);
				else if (PREF_COLORING.equals(event.getProperty()))
					coloring = pStore.getBoolean(PREF_COLORING);
				else if (PREF_VALENCES_VISIBLE.equals(event.getProperty()))
					valencesVisible = pStore.getBoolean(PREF_VALENCES_VISIBLE);
				else if (PREF_MOL_IMAGE_WIDTH.equals(event.getProperty()))
					molImageWidth = pStore.getInt(PREF_MOL_IMAGE_WIDTH);
				else if (PREF_MOL_IMAGE_HEIGHT.equals(event.getProperty()))
					molImageHeight = pStore.getInt(PREF_MOL_IMAGE_HEIGHT);
				else if (PREF_ENABLE_RENDERER.equals(event.getProperty()))
					enableRendering = pStore.getBoolean(PREF_ENABLE_RENDERER);
				else if (PREF_LABEL_MODE.equals(event.getProperty()))
					labelMode = pStore.getString(PREF_LABEL_MODE);
				else if (PREF_SUPERATOM_MODE.equals(event.getProperty()))
					superatomMode = pStore.getString(PREF_SUPERATOM_MODE);
				else if (PREF_BACKGROUND_COLOR.equals(event.getProperty()))
					backgroundColor = pStore.getString(PREF_BACKGROUND_COLOR);
				else if (PREF_BASE_COLOR.equals(event.getProperty()))
					baseColor = pStore.getString(PREF_BASE_COLOR);
			}
		});

		enableRendering = pStore.getBoolean(PREF_ENABLE_RENDERER);
		bondLength = pStore.getInt(PREF_BOND_LENGTH);
		showImplicitHydrogens = pStore.getBoolean(PREF_SHOW_IMPLICIT_HYDROGENS);
		valencesVisible = pStore.getBoolean(PREF_VALENCES_VISIBLE);
		coloring = pStore.getBoolean(PREF_COLORING);
		molImageWidth = pStore.getInt(PREF_MOL_IMAGE_WIDTH);
		molImageHeight = pStore.getInt(PREF_MOL_IMAGE_HEIGHT);
		labelMode = pStore.getString(PREF_LABEL_MODE);
		superatomMode = pStore.getString(PREF_SUPERATOM_MODE);
		backgroundColor = pStore.getString(PREF_BACKGROUND_COLOR);
		baseColor = pStore.getString(PREF_BASE_COLOR);
	}

	@Override
	public void stop(final BundleContext context) throws Exception {
		super.stop(context);
		plugin = null;
	}

	public static IndigoPlugin getDefault() {
		return plugin;
	}

	public float bondLength() {
		return bondLength;
	}

	public boolean showImplicitHydrogens() {
		return showImplicitHydrogens;
	}

	public boolean coloring() {
		return coloring;
	}

	public String labelMode() {
		return labelMode;
	}

	public String superatomMode() {
		return superatomMode;
	}

	public int molImageWidth() {
		return molImageWidth;
	}

	public int molImageHeight() {
		return molImageHeight;
	}

	public boolean isRenderingEnabled() {
		return enableRendering;
	}

	public boolean isValencesVisible() {
		return valencesVisible;
	}

	public String backgroundColor() {
		return normalizeColor(backgroundColor);
	}

	public String baseColor() {
		return normalizeColor(baseColor);
	}
}
