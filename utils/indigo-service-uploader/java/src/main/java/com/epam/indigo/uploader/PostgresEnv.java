package com.epam.indigo.uploader;

import org.apache.logging.log4j.LogManager;

import java.io.IOException;
import java.sql.*;
import java.util.Properties;


public class PostgresEnv {

   public static void dropCreateTable(String table_name) {
      try {
         PostgresEnv.getStatement().executeUpdate("DROP TABLE IF EXISTS " + table_name);
         PostgresEnv.getStatement().executeUpdate("CREATE TABLE " + table_name + "(s serial, m bytea, p jsonb NOT NULL DEFAULT '{}')");
      } catch (SQLException ex) {
         LogManager.getLogger("PostgresEnv").error("Couldn't create a table!", ex);
      }
   }

   public static void createBingoIndex(String table_name) {
      String index_name = table_name + "_idx";
      try {
         getStatement().executeUpdate("CREATE INDEX " + index_name + " ON " + table_name + " USING bingo_idx (m bingo.bmolecule) with (IGNORE_STEREOCENTER_ERRORS=1,IGNORE_CISTRANS_ERRORS=1,FP_TAU_SIZE=0)");
      } catch (SQLException ex) {
         LogManager.getLogger("PostgresEnv").error("Couldn't create an index!", ex);
      }
   }
   private Connection _connection = null;
   private Properties _parameters = null;


   private PostgresEnv() {
      try {
         Class.forName("org.postgresql.Driver");
      } catch (ClassNotFoundException cnfe) {
         LogManager.getLogger("PostgresEnv").error("Couldn't find the driver!", cnfe);
         System.out.println("PostgreSQL driver not found");
         System.exit(2);
      }

      LogManager.getLogger("PostgresEnv").info("Registered the driver OK.");

      try {
         _parameters = new Properties();
         _parameters.loadFromXML(getClass().getResourceAsStream("/database.xml"));
         _connection = DriverManager.getConnection(_parameters.getProperty("db_url"), _parameters);
      } catch (IOException | SQLException se) {
         LogManager.getLogger(PostgresEnv.class).error("Couldn't connect: print out a stack trace and exit.", se);
         System.exit(1);
      }

      if (_connection != null) {
         LogManager.getLogger(PostgresEnv.class.getName()).info("Successfully connected to a database");
      } else {
         LogManager.getLogger(PostgresEnv.class.getName()).error("We should never get here.");
      }
   }


   public static String getSchemaName() {
      return getInstance()._parameters.getProperty("schema_name");
   }
   public static String getDataDir() {
      return getInstance()._parameters.getProperty("data_dir");
   }

   public static PostgresEnv getInstance() {
      return PostgresEnvHolder.INSTANCE;
   }

   public static Connection getConnection() {
      return getInstance()._connection;
   }

   public static Statement getStatement() {
      Statement res = null;
      try {
         res = getInstance()._connection.createStatement();
      } catch (SQLException ex) {
         LogManager.getLogger(PostgresEnv.class.getName()).error(ex);
      }
      return res;
   }

   private static class PostgresEnvHolder {
      private static final PostgresEnv INSTANCE = new PostgresEnv();
   }
}
