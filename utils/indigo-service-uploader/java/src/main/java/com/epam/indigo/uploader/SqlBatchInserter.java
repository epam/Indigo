package com.epam.indigo.uploader;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.AbstractQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.zip.GZIPOutputStream;

import org.apache.logging.log4j.LogManager;
import org.postgresql.util.PGobject;


public class SqlBatchInserter {

   private final String _tableName;
   public static final int BATCH_SIZE = 200;

   public SqlBatchInserter(String table_name) {
      this._tableName = table_name;
   }


   private static class SDWorker implements Runnable {
      private static final ThreadLocal<JsonParser> parser = new ThreadLocal<>();
      private final AbstractQueue<SdfIterator.SDItem> sdQueue;
      private final String str;

      private SDWorker(AbstractQueue<SdfIterator.SDItem> sdQueue, String str) {
         this.sdQueue = sdQueue;
         this.str = str;
      }

      @Override
      public void run() {
         ByteArrayOutputStream buf = new ByteArrayOutputStream();
         if(parser.get() == null) {
            parser.set(new JsonParser());
         }
         try {
            SdfIterator.SDItem sdItem = SdfIterator.createItem(str);
            try (OutputStreamWriter sd = new OutputStreamWriter(new GZIPOutputStream(buf))) {
               sd.append(sdItem.mol);
            } catch (IOException ex) {
               LogManager.getRootLogger().error(ex.getMessage(), ex);
            }
            String paramsJson = parser.get().parseParametersIntoJson(sdItem.props);
            PGobject dataObject = new PGobject();
            dataObject.setType("jsonb");
            dataObject.setValue(paramsJson);

            sdItem.jsonObject = dataObject;
            sdItem.buf = buf.toByteArray();

            sdQueue.add(sdItem);

         } catch (SQLException ex) {
         }
      }

   }

   private class PgBatchInserter implements Runnable {
      private final AtomicBoolean allProcessed;
      private final AtomicInteger numberProcesed;
      private final LinkedBlockingDeque<SdfIterator.SDItem> sdQueue;

      private PgBatchInserter(AtomicBoolean allProcessed, AtomicInteger numberProcesed, LinkedBlockingDeque<SdfIterator.SDItem> sdQueue) {
         this.allProcessed = allProcessed;
         this.numberProcesed = numberProcesed;
         this.sdQueue = sdQueue;
      }
      @Override
      public void run() {
         int localProcessed = 0;
         try (PreparedStatement ps = PostgresEnv.getConnection().prepareStatement("INSERT INTO " + _tableName + "(m,p) VALUES (?,?)")) {
            while (!Thread.currentThread().isInterrupted() && (!allProcessed.get() || (numberProcesed.get() != localProcessed))) {
               try {
                  SdfIterator.SDItem sd = sdQueue.takeFirst();
                  ps.setBytes(1, sd.buf);
                  ps.setObject(2, sd.jsonObject);
                  ps.addBatch();

                  localProcessed++;
                  if (localProcessed % BATCH_SIZE == 0) {
                     ps.executeBatch();
                  }
               } catch (InterruptedException ex) {
               }
            }
            ps.executeBatch();
         } catch (SQLException ex) {
            LogManager.getRootLogger().error(ex.getMessage(), ex);
         }
      }

   }

   int process(InputStream molScanner) {
      return process(molScanner, -1);
   }
   int processParallel(InputStream molScanner) {
      return processParallel(molScanner, -1);
   }
   int processParallel(InputStream molScanner, int max_num) {
      try {
         int iter_num = 0;

         final AtomicBoolean allProcessed = new AtomicBoolean(false);
         final AtomicInteger numberProcesed = new AtomicInteger(0);

         final LinkedBlockingDeque<SdfIterator.SDItem> sdQueue = new LinkedBlockingDeque<>();

         Thread sqlInserter = new Thread(new PgBatchInserter(allProcessed, numberProcesed, sdQueue));

         sqlInserter.start();
//         ExecutorService exec = Executors.newCachedThreadPool();
         ExecutorService exec = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors()) ;
         SdfIterator sdIter = new SdfIterator(molScanner);
         for (String str : sdIter.delimIter()) {
            exec.submit(new SDWorker(sdQueue, str));
            iter_num++;
            if(iter_num % 1000 == 0) {
               LogManager.getRootLogger().info("Processed " + iter_num);
            }
            if (max_num > 0 && iter_num > max_num) {
               break;
            }
         }
         numberProcesed.compareAndSet(0, iter_num);
         allProcessed.compareAndSet(false, true);
         sqlInserter.join();

         exec.shutdown();
         while (!exec.isTerminated()) {
         }

         return iter_num;
      } catch (InterruptedException ex) {
      }
      return 0;
   }


   int process(InputStream molScanner, int max_num) {
      ByteArrayOutputStream buf = new ByteArrayOutputStream();
      StringBuilder params = new StringBuilder();
      JsonParser parser = new JsonParser();

      long other, other_time = 0, batch_time = 0;
      int iter_num = 0;
      try (PreparedStatement ps = PostgresEnv.getConnection().prepareStatement("INSERT INTO " + _tableName + "(m,p) VALUES (?,?)")) {
         SdfIterator sdIter = new SdfIterator(molScanner);
         for (SdfIterator.SDItem str : sdIter) {
            other = System.currentTimeMillis();
            buf.reset();
            params.delete(0, params.length());
            try (OutputStreamWriter sd = new OutputStreamWriter(new GZIPOutputStream(buf))) {
               sd.append(str.mol);
            } catch (IOException ex) {
               LogManager.getRootLogger().error(ex.getMessage(), ex);
            }
            params.append(parser.parseParametersIntoJson(str.props));

            PGobject dataObject = new PGobject();
            dataObject.setType("jsonb");
            dataObject.setValue(params.toString());

            other_time += (System.currentTimeMillis() - other);

            ps.setBytes(1, buf.toByteArray());
            ps.setObject(2, dataObject);
            ps.addBatch();
            if (iter_num % BATCH_SIZE == 0) {
               long batch_start = System.currentTimeMillis();
               ps.executeBatch();
               batch_time += (System.currentTimeMillis() - batch_start);
               LogManager.getLogger("").info("Processed " + iter_num);
            }
            iter_num++;
            if (max_num > 0 && iter_num > max_num) {
               break;
            }
         }
         ps.executeBatch();
      } catch (SQLException ex) {
         LogManager.getRootLogger().error(ex.getMessage(), ex);
      }

      LogManager.getLogger("").info("Batch time = " + batch_time + " ms");
      LogManager.getLogger("").info("Other time = " + other_time + " ms");
      return iter_num;
   }

}
