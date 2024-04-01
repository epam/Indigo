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

package com.epam.indigo.knime.fpsim;

import org.knime.core.node.defaultnodesettings.SettingsModelString;

import com.epam.indigo.knime.common.IndigoNodeSettings;
import com.epam.indigo.knime.common.SettingsModelFloat;

public class IndigoFingerprintSimilaritySettings extends IndigoNodeSettings
{
   public static final int TARGET_PORT = 0;
   public static final int QUERY_PORT = 1;
   
   enum Metric
   {
      Tanimoto, EuclidSub, Tversky
   }

   enum Aggregation
   {
      Minimum, Maximum, Average
   }
   
   public final SettingsModelString targetColumn = new SettingsModelString("colName", null);
   public final SettingsModelString queryColumn = new SettingsModelString("colName2", null);
   public final SettingsModelString newColName = new SettingsModelString("newColName", "similarity");
   public final SettingsModelString metric = new SettingsModelString("metric", Metric.Tanimoto.toString());
   public final SettingsModelFloat tverskyAlpha = new SettingsModelFloat("tverskyAlpha", 0.5f);
   public final SettingsModelFloat tverskyBeta = new SettingsModelFloat("tverskyBeta", 0.5f);
   public final SettingsModelString aggregation = new SettingsModelString("aggregation", Aggregation.Average.toString());

   
   public IndigoFingerprintSimilaritySettings() {
      addSettingsParameter(targetColumn);
      addSettingsParameter(queryColumn);
      addSettingsParameter(newColName);
      addSettingsParameter(metric);
      addSettingsParameter(aggregation);
      addSettingsParameter(tverskyAlpha);
      addSettingsParameter(tverskyBeta);
   }



}
