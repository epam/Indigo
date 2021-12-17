package com.epam.indigo.uploader;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import java.util.Properties;
import java.util.regex.Pattern;
import org.apache.logging.log4j.LogManager;

/**
 *
 */
public class JsonParser {

   private final Gson gson = new Gson();
   private final Pattern numberPattern = Pattern.compile("[-]?[0-9]*\\.?[0-9]+");

   public JsonParser() {
   }

   public boolean isNumeric(String str) {
      return numberPattern.matcher(str).matches();
   }

   public static double getNumber(String str) throws NumberFormatException {
      return Double.parseDouble(str);
   }

   public JsonElement getPropertValue(String val) {
      if(isNumeric(val)) {
         try {
            return new JsonPrimitive(getNumber(val));
         } catch (NumberFormatException e) {
            LogManager.getLogger(JsonParser.class.getName()).warn(e.getMessage());
         }
      }

      return new JsonPrimitive(val.toLowerCase());
   }

   public String parseParametersIntoJson(Properties props) {
      JsonArray jsonProps = new JsonArray();
      for (String p_name : props.stringPropertyNames()) {
         String p_val = props.getProperty(p_name);
         JsonObject elem = new JsonObject();

         elem.addProperty("x", p_name.trim().toLowerCase());
         elem.add("y", getPropertValue(p_val));

         elem.addProperty("a", p_name);
         elem.addProperty("b", p_val);

         jsonProps.add(elem);
      }
      return gson.toJson(jsonProps);
   }
//   public String parseParametersToJson(Gson gson, Properties props) {
//      JsonArray jsonProps = new JsonArray();
//      for (String p_name : props.stringPropertyNames()) {
//         String p_val = props.getProperty(p_name);
//         JsonObject elem = new JsonObject();
//
//         elem.addProperty("idx_k", p_name.trim().toLowerCase());
//         elem.add("idx_v", getPropertValue(p_val));
//
//         elem.addProperty("org_k", p_name);
//         elem.addProperty("org_v", p_val);
//
//         jsonProps.add(elem);
//      }
//      return gson.toJson(jsonProps);
//   }
}
