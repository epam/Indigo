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

package com.epam.indigo.knime.standardizer;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.knime.core.node.NodeLogger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class IndigoStandardizerUtils {
   /**
    *  An auxiliary class to upload options data from xml-file and keep it.
    */

   private static final NodeLogger LOGGER = NodeLogger
         .getLogger(IndigoStandardizerUtils.class);
   
   // this map is initialized at the first class call and keeps pairs [Option name; Option object]
   public static Map<String, Option> optionsMap;

   static {
      optionsMap = loadOptions();
   }
   
   public static class Option {
      /**
       * Class for keeping data of an option
       * 
       * scopes values: 
       * 0 - applicable for a molecule and a query molecule 
       * 1 - applicable for a molecule only 
       * 2 - applicable for a query molecule only
       * 3 - applicable for QueryMolecule type only with non-zero coordinates
       */
      private String group;
      private String name;
      private String description;
      private byte scope;

      public Option(String group, String name, String description, byte scope) {
         this.group = group;
         this.name = name;
         this.description = description;
         this.scope = scope;

      }

      public String getGroup() {
         return group;
      }

      public String getName() {
         return name;
      }

      public String getDescription() {
         return description;
      }

      public int getScope() {
         return scope;
      }
      
      @Override
      public String toString() {
         return getName();
      }
   }
   
   static private Map<String, Option> loadOptions() {
      /**
       * Load options from xml-file into map<Option name, Option object> and returns the map.
       */

      Map<String, Option> result = new HashMap<String, Option>();
      
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      try {
         DocumentBuilder db = dbf.newDocumentBuilder();
         Document doc = db.parse(IndigoStandardizerUtils.class.getResourceAsStream(
               "IndigoStandardizerOptions.xml"));
         Element docElement = doc.getDocumentElement();
         NodeList groups = docElement.getElementsByTagName("Group");
         int groupsCount = groups.getLength();
         // go through option's groups
         for (int i = 0; i < groupsCount; i++) {
            Element group = (Element) groups.item(i);
            String groupName = group.getAttribute("name");
            NodeList opts = group.getElementsByTagName("Option");
            int optionsCount = opts.getLength();
            // go through options
            for (int j = 0; j < optionsCount; j++) {
               Element opt = (Element) opts.item(j);
               String optionsName = opt.getAttribute("name");
               String description = opt.getElementsByTagName("Description")
                     .item(0).getTextContent();
               Byte scope = Byte.parseByte(opt.getElementsByTagName("Scope")
                     .item(0).getTextContent());
               result.put(optionsName, new Option(groupName, optionsName, description, scope));
            }
         }
      } catch (ParserConfigurationException e) {
         LOGGER.error(e);
         e.printStackTrace();
      } catch (SAXException e) {
         LOGGER.error(e);
         e.printStackTrace();
      } catch (IOException e) {
         LOGGER.error(e);
         e.printStackTrace();
      }
      
      return result;
   }
   
   /**
    * To get name list of options with a scope
    * @param scope An integer value (0, 1, 2 and 3)
    * @return Name list of options with the scope
    */
   public static List<String> getOptionsByScope(int scope) {
      List<String> inappropriate  = new ArrayList<String>();
      Collection<Option> allOptions  = optionsMap.values();
      for (Option option: allOptions) {
         if (option.getScope() == scope) {
            // scope definition see in xml file
            inappropriate.add(option.getName());
         }
      }
      return inappropriate;
   }
   
}
