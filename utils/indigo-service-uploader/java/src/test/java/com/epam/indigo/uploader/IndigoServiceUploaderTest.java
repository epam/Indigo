package com.epam.indigo.uploader;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.zip.GZIPInputStream;
import static org.junit.Assert.*;
import org.junit.*;
import org.junit.rules.TestName;



public class IndigoServiceUploaderTest {
   public static final String TEST_SCHEME = "test_upload";
   
   @Rule public final TestName name = new TestName();
   
//   @BeforeClass
//   public static void init() throws SQLException {
//      PostgresEnv.getStatement().executeUpdate("DROP SCHEMA IF EXISTS " + TEST_SCHEME + "  CASCADE");
//      PostgresEnv.getStatement().executeUpdate("CREATE SCHEMA " + TEST_SCHEME);
//   }

   public IndigoServiceUploaderTest() {
   }

   @Test
   public void testDatabaseBingoVersion() throws SQLException {
      String sqlBuilder = "select bingo.getversion()";
      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
      int str_num = 0;
      while (resultSet.next()) {
         ++str_num;
      }
      assertEquals(1, str_num);
   }

   @Test
   public void testBasicSDIterator() throws FileNotFoundException  {
      SdfIterator sd = new SdfIterator(new FileInputStream("data/test_pubchem_10.sdf"));
      int x = 0, p=0;

      for(SdfIterator.SDItem str : sd) {
         ++x;
         p += str.props.size();
      }
      assertEquals(10, x);
      assertEquals(325, p);
   }
   @Test
   public void testBasicSDScope() throws IOException {
      try (InputStream test = new GZIPInputStream(new FileInputStream("data/test-18.sd.gz"))) {
         SdfIterator sd = new SdfIterator(test);
         int x = 0, p = 0;
         String last_mol = "";

         for (SdfIterator.SDItem str : sd) {
            ++x;
            p += str.props.size();
            last_mol = str.mol;
         }
         assertEquals(18, x);
         assertEquals(576, p);
         assertTrue(last_mol.startsWith("StarDropID 3"));
      }
      try (InputStream test = new GZIPInputStream(new FileInputStream("data/test-108.sd.gz"))) {
         SdfIterator sd = new SdfIterator(test);
         int x = 0, p = 0;

         for (SdfIterator.SDItem str : sd) {
            ++x;
            p += str.props.size();
         }
         assertEquals(108, x);
         assertEquals(3456, p);
      }
      try (InputStream test = new GZIPInputStream(new FileInputStream("data/test-2759.sd.gz"))) {
         SdfIterator sd = new SdfIterator(test);
         int x = 0, p = 0;

         for (SdfIterator.SDItem str : sd) {
            ++x;
            p += str.props.size();
         }
         assertEquals(2759, x);
         assertEquals(8277, p);
      }
   }

   @Test
   public void testSDInsertBasic() throws IOException, SQLException {
      String table_name = TEST_SCHEME + "." + name.getMethodName();
      PostgresEnv.dropCreateTable(table_name);
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (FileInputStream molScanner = new FileInputStream("data/test_pubchem_10.sdf")) {
         insert.process(molScanner);
      }

      String sqlBuilder = "select elems->>'a',elems->>'b' from " +
              table_name +
              ",jsonb_array_elements(p) elems where elems->>'x' like '%mass%' and (elems->>'y')::float > 300";

      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
      int str_num = 0;
      while (resultSet.next()) {
         ++str_num;
      }
      assertEquals(6, str_num);
   }

   @Test
   public void testSDInsertParallelBasic() throws IOException, SQLException {
      String table_name = TEST_SCHEME + "." + name.getMethodName();
      PostgresEnv.dropCreateTable(table_name);
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (FileInputStream molScanner = new FileInputStream("data/test_pubchem_10.sdf")) {
         insert.processParallel(molScanner);
      }

      String sqlBuilder = "select elems->>'a',elems->>'b' from " +
              table_name +
              ",jsonb_array_elements(p) elems where elems->>'x' like '%mass%' and (elems->>'y')::float > 300";

      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
      int str_num = 0;
      String last_key = "";
      String last_val = "";
      while (resultSet.next()) {
         ++str_num;
         last_key = resultSet.getString(1);
         last_val = resultSet.getString(2);
      }
      assertEquals(6, str_num);
      assertEquals("PUBCHEM_EXACT_MASS", last_key);
   }
   @Test
   public void testSDInsertMaybridge() throws IOException, SQLException {
      String table_name = TEST_SCHEME + "." + name.getMethodName();
      PostgresEnv.dropCreateTable(table_name);
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (GZIPInputStream molScanner = new GZIPInputStream(new FileInputStream("data/maybridge-stardrop-sample.sd.gz"))) {
         insert.processParallel(molScanner);
      }

      String sqlBuilder = "select elems->>'a',elems->>'b' from " +
              table_name +
              ",jsonb_array_elements(p) elems where elems->>'x' like '%logp%' and (elems->>'y')::float > 5";

      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
      int str_num = 0;
      String last_key = "";
      String last_val = "";
      while (resultSet.next()) {
         ++str_num;
         last_key = resultSet.getString(1);
         last_val = resultSet.getString(2);
      }
      assertEquals(16, str_num);
      assertEquals("logP", last_key);
      resultSet = PostgresEnv.getStatement().executeQuery("select * from " + table_name);
      str_num = 0;
      while (resultSet.next()) {
         ++str_num;
      }
      assertEquals(108, str_num);
   }
   @Test
   public void testSDInsertParrallelCorrect18() throws IOException, SQLException {
      String table_name = TEST_SCHEME + "." + name.getMethodName();
      PostgresEnv.dropCreateTable(table_name);
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (GZIPInputStream molScanner = new GZIPInputStream(new FileInputStream("data/test-18.sd.gz"))) {
         insert.processParallel(molScanner);
      }

      String sqlBuilder = "select bingo.checkmolecule(m) from  " +
              table_name;

      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
      int str_num = 0;
      String last_key;
      while (resultSet.next()) {
         ++str_num;
         last_key = resultSet.getString(1);
         assertNull(last_key);
      }
      assertEquals(18, str_num);
   }
   @Test
   public void testSDFloatNumbers() throws IOException, SQLException {
      String table_name = TEST_SCHEME + "." + name.getMethodName();
      PostgresEnv.dropCreateTable(table_name);
      SqlBatchInserter insert = new SqlBatchInserter(table_name);
      try (FileInputStream molScanner = new FileInputStream("data/test-18-floats.sdf")) {
         insert.processParallel(molScanner);
      }
      {
         String sqlBuilder = "select elems->>'a',elems->>'b' from " +
                 table_name +
                 ",jsonb_array_elements(p) elems where elems->>'x' like '%logs%' and (elems->>'y')::float > 0.5";

         ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
         int str_num = 0;
         int contains = 0;
         while (resultSet.next()) {
            ++str_num;
            contains += resultSet.getString(2).contains("777") ? 1 : 0;
         }
         assertEquals(36, str_num);
         assertEquals(1, contains);
      }
      {
         String sqlBuilder = "select elems->>'a',elems->>'b' from " +
                 table_name +
                 ",jsonb_array_elements(p) elems where elems->>'x' like '%logs%' and (elems->>'y')::float > 1";

         ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder);
          int str_num = 0;
         int contains = 0;
         while (resultSet.next()) {
            ++str_num;
            contains += resultSet.getString(2).contains("777") ? 1 : 0;
         }
         assertEquals(26, str_num);
         assertEquals(0, contains);
      }
   }
//   @Test
//   public void testSDInsertShrodinger() throws FileNotFoundException, IOException, SQLException {
//      String table_name = "test_unit_shrodinger";
//      PostgresEnv.dropCreateTable(table_name);
//      SqlBatchInserter insert = new SqlBatchInserter(table_name);
//      try (GZIPInputStream molScanner = new GZIPInputStream(new FileInputStream("data/test_from_schrodinger.sd.gz"))) {
//         insert.processParallel(molScanner);
//      }
//      
//      PostgresEnv.createBingoIndex(table_name);
//
//      StringBuilder sqlBuilder = new StringBuilder();
//      sqlBuilder.append("select elems->>'a',elems->>'b' from ")
//              .append(table_name)
//              .append(",jsonb_array_elements(p) elems where elems->>'x' like '%flexibility%' and (elems->>'y')::float > 0.3");
//
//      ResultSet resultSet = PostgresEnv.getStatement().executeQuery(sqlBuilder.toString());
//      int str_num = 0;
//      String last_key = "";
//      String last_val = "";
//      while (resultSet.next()) {
//         ++str_num;
//         last_key = resultSet.getString(1);
//         last_val = resultSet.getString(2);
//      }
//      assertEquals(2, str_num);
//      assertEquals("Flexibility", last_key);
//   }
   
}
