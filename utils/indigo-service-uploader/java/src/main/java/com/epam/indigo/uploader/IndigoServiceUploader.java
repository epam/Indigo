package com.epam.indigo.uploader;

import org.apache.logging.log4j.LogManager;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.GZIPInputStream;

public class IndigoServiceUploader {
   public static void main(String[] args) {
      if (args.length != 2) {
        System.out.println("Usage: uploader <file_path> <table_name>");
        return;
      }
      
      /*
      * Detect single or multithreaded
      */
      boolean multiThreaded = Runtime.getRuntime().availableProcessors() > 1;
      startUpload(args[0], args[1], multiThreaded);
   }


   /*
    * Best time
    * 3200 structures per second insert
    * 1500 structures per second insert with indexing
   */
   public static int startUpload(String path, String table_name, boolean multiThreaded) {
//      int max_num = 10000;
      int str_num = 0;
      int max_num = -1;

      long total_time = System.currentTimeMillis();
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (InputStream molScanner = new GZIPInputStream(new FileInputStream(path))) {
         if(multiThreaded)
            str_num = insert.processParallel(molScanner, max_num);
         else
            str_num = insert.process(molScanner, max_num);
      } catch (IOException ex) {
         LogManager.getRootLogger().error(ex.getMessage(), ex);
         System.exit(0);
      }

//      PostgresEnv.createBingoIndex(table_name);
      total_time = (System.currentTimeMillis() - total_time);

      LogManager.getLogger("").info("Insert total time = " + total_time + " ms");
      LogManager.getLogger("").info("Average insert time = " + (int)((double)str_num / total_time * 1000.0) + " structures per second");
      LogManager.getLogger("").info("Total structures processed = " + str_num);
      return str_num;
   }
   
   

//   public static void importPG_1() {
//         StringBuilder sqlBuilder = new StringBuilder();
//         sqlBuilder.append("select bingo.getversion()");
//         ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder.toString());
//         int str_num = 0;
//         while (resultSet.next()) {
//            System.out.println(resultSet.getString(1));
//            ++str_num;
//         }
//      String table_name = "test_celgene";
//      // PostgresEnv.dropCreateTable(table_name);
//
////         Indigo indigo = new Indigo();
//      int x = 0;
////      int max_num = 10000;
//      int max_num = -1;
//
//      long total_time = System.currentTimeMillis();
//      SqlBatchInserter insert = new SqlBatchInserter(table_name);
//      try (FileInputStream molScanner = new FileInputStream("/mnt/ramdisk/tmp/pubchem_100k.sd")) {
//         x = insert.process(molScanner, max_num);
//      } catch (FileNotFoundException ex) {
//         Logger.getRootLogger().error(ex.getMessage(), ex);
//         System.exit(0);
//      } catch (IOException ex) {
//         Logger.getRootLogger().error(ex.getMessage(), ex);
//         System.exit(0);
//      }
//      total_time = (System.currentTimeMillis() - total_time);
//
//      Logger.getLogger("").info("Insert total time = " + total_time + " ms");
//      Logger.getLogger("").info("Average insert time = " + (int)((double)x / total_time * 1000.0) + " structures per second");
//      Logger.getLogger("").info("Total structures processed = " + x);
//
////            start = System.currentTimeMillis();
////         createBingoIndex(table_name);
////            Logger.getLogger(IndigoServiceUploader.class).info("Index time = " + (System.currentTimeMillis() - start) + " ms");
//   }
//
//   private static void addJsonValue(StringBuilder params, String val) {
//      if (val.contains("\n")) {
//         StringTokenizer st = new StringTokenizer(val, "\n");
//         boolean started = false;
//         params.append("[");
//         while (st.hasMoreTokens()) {
//            if (started) {
//               params.append(",");
//            }
//            addJsonSimpleValue(params, st.nextToken());
//            started = true;
//         }
//         params.append("]");
//      } else {
//         addJsonSimpleValue(params, val);
//      }
//   }
//   private static void addJsonSimpleValue(StringBuilder params, String val) {
//      if (isNumeric(val)) {
//         String nVal = val;
//         String tmpVal;
//         while(nVal.startsWith("0") && nVal.length() > 1 && nVal.charAt(1) != '.') {
//            tmpVal = nVal.substring(1);
//            nVal = tmpVal;
//         }
//         params.append(nVal).append("");
//      } else {
//         params.append("\"").append(val.replace("\\", "\\\\").replace("\"", "\\\"").replace("\'", "\\\"")).append("\"");
//      }
//   }
//   private static void addParameters(String mol, StringBuilder params) {
//      params.append("{");
//      try (Scanner pScanner = new Scanner(new ByteArrayInputStream(mol.getBytes()))) {
//         pScanner.useDelimiter(java.util.regex.Pattern.quote("\n> <"));
//         /*
//          * Skip first
//          */
//         if (pScanner.hasNext()) {
//            pScanner.next();
//         }
//
//         while (pScanner.hasNext()) {
//            String[] pLines = pScanner.next().trim().split("\n");
//            if(pLines.length > 0) {
//               String p_name = pLines[0].substring(0, pLines[0].lastIndexOf(">"));
//
//               if (params.length() > 1) {
//                  params.append(",");
//               }
//               params.append("\"").append(p_name).append("\":");
//
//               addJsonValues(params, pLines);
//
//            }
//         }
//      }
//      params.append("}");
//   }
//   private static void testProps() {int x = 0;
//       int max_num = 1;
//      int batch_size = 200;
//      long batch_time = 0, other_time = 0, start, other = 0;
//
//      start = System.currentTimeMillis();
//      ByteArrayOutputStream buf = new ByteArrayOutputStream();
//      StringBuilder params = new StringBuilder();
//
//      try (Scanner molScanner = new Scanner(new BufferedReader(new FileReader("/mnt/ramdisk/tmp/pubchem_100k.sd")))) {
//         molScanner.useDelimiter(java.util.regex.Pattern.quote("$$$$\n"));
//         while (molScanner.hasNext()) {
//            String mol = molScanner.next();
////            for (IndigoObject mol : indigo.iterateSDFile("/mnt/ramdisk/tmp/pubchem_100k.sd")) {
//            other = System.currentTimeMillis();
//            buf.reset();
//            params.delete(0, params.length());
//            try (OutputStreamWriter sd = new OutputStreamWriter(new GZIPOutputStream(buf))) {
////            try (OutputStreamWriter sd = new OutputStreamWriter(buf)){
//               sd.append(mol);
//            } catch (IOException ex) {
////               Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//            }
//            addParameters(mol, params);
//
//            System.out.println(params.toString());
////            System.out.println(mol);
//            if (x++ > max_num) {
//               break;
//            }
//         }
//      } catch (FileNotFoundException ex) {
////         Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//            }
//   }
//   private static void addJsonValues(StringBuilder params, String[] pLines) {
//      if (pLines.length > 2) {
//         params.append("[");
//         for(int i = 2; i < pLines.length; ++i) {
//            if (i > 2) {
//               params.append(",");
//            }
//            addJsonSimpleValue(params, pLines[i]);
//         }
//         params.append("]");
//      } else {
//         if(pLines.length > 1) {
//            addJsonSimpleValue(params, pLines[1]);
//         } else {
//            params.append("");
//         }
//      }
//   }
//   private static void addParameters(Properties props, StringBuilder params) {
//      params.append("{");
//      for (String p_name : props.stringPropertyNames()) {
//         if (params.length() > 1) {
//            params.append(",");
//         }
//         params.append("\"").append(p_name).append("\":");
//         addJsonSimpleValue(params, props.getProperty(p_name));
//      }
//      params.append("}");
//   }
   //   public static void selectSubSet() {
//      Indigo indigo = new Indigo();
//      int x = 0;
//      int max_num = 10000;
//      try (BufferedWriter sd = new BufferedWriter(new OutputStreamWriter(new FileOutputStream("/mnt/ramdisk/tmp/pubchem_10k.sd")))) {
//         for (IndigoObject mol : indigo.iterateSDFile("/mnt/ramdisk/tmp/pubchem_1M_sd.gz")) {
//            sd.append(mol.rawData());
//            sd.newLine();
//            sd.append("$$$$");
//            sd.newLine();
//            if (x++ > max_num) {
//               break;
//            }
//         }
//      } catch (FileNotFoundException ex) {
//         Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//      } catch (IOException ex) {
//         Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//      }
//   }
//   public static void convertToOneGz() {
//      Indigo indigo = new Indigo();
//
//      String sd_path = "/mnt/ramdisk/tmp/PubChem.gz/";
//      try (BufferedWriter gz = new BufferedWriter(new OutputStreamWriter(new GZIPOutputStream(new FileOutputStream(sd_path + "../pubchem_1M_sd.gz"))))) {
//         File f = new File(sd_path);
//         for (File fs : f.listFiles()) {
//            System.out.println("Process " + fs.getAbsolutePath());
//            for (IndigoObject mol : indigo.iterateSDFile(fs.getAbsolutePath())) {
//               gz.append(mol.rawData());
//               gz.newLine();
//               gz.append("$$$$");
//               gz.newLine();
//            }
//         }
//      } catch (FileNotFoundException ex) {
//         Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//      } catch (IOException ex) {
//         Logger.getLogger(IndigoTools.class.getName()).log(Level.SEVERE, null, ex);
//      }
//   }
}


