/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 * 
 * This file is part of Indigo toolkit.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

package com.epam.indigo;
import com.sun.jna.Native;
import java.io.File;
import java.io.IOException;

public class Bingo {
	private Indigo _indigo;
   	private static BingoLib _lib;
	private int _id;

	public Bingo (Indigo indigo, String location, String type, String options) {
        loadLibrary(indigo.getUserSpecifiedPath());
         _id = Bingo.checkResult(indigo, _lib.bingoCreateDatabaseFile(location, type, options));
        _indigo = indigo;
	}

    public Bingo (Indigo indigo, String location, String options) {
        loadLibrary(indigo.getUserSpecifiedPath());
        _id = Bingo.checkResult(indigo, _lib.bingoLoadDatabaseFile(location, options));
        _indigo = indigo;
    }


    protected void finalize() {
		dispose();
	}

	private synchronized static void loadLibrary (String path) {
		if (_lib != null)
			return;

		int os = Indigo.getOs();

		if (os == Indigo.OS_LINUX || os == Indigo.OS_SOLARIS)
			_lib = (BingoLib)Native.loadLibrary(getPathToBinary(path, "libbingo.so"), BingoLib.class);
		else if (os == Indigo.OS_MACOS)
			_lib = (BingoLib)Native.loadLibrary(getPathToBinary(path, "libbingo.dylib"), BingoLib.class);
		else // os == OS_WINDOWS
				_lib = (BingoLib)Native.loadLibrary(getPathToBinary(path, "bingo.dll"), BingoLib.class);
	}

    private static String getPathToBinary (String path, String filename) {
        String dllpath = Indigo.getPlatformDependentPath();

        if (path == null) {
            String res = Indigo.extractFromJar(Bingo.class, "/" + dllpath, filename);
            if (res != null)
                return res;
            path = "lib";
        }
        path = path + File.separator + dllpath + File.separator + filename;
        try {
            return (new File(path)).getCanonicalPath();
        } catch (IOException e) {
            return path;
        }
    }

	public void dispose() {
		if (_id >= 0) {
			_indigo.setSessionID();
			Bingo.checkResult(_indigo, _lib.bingoCloseDatabase(_id));
			_id = -1;
		}
	}

	public void close() {
		dispose();
	}

	public static int checkResult(Indigo indigo, int result) {
		if (result < 0) {
	        throw new BingoException(indigo, indigo.getLibrary().indigoGetLastError());
	    }

	    return result;
	}

    public static float checkResult(Indigo indigo, float result) {
	    if (result < 0.0) {
	        throw new BingoException(indigo, indigo.getLibrary().indigoGetLastError());
	    }

	    return result;
	}

    public static String checkResult(Indigo indigo, String result) {
        if (result == null) {
            throw new BingoException(indigo, indigo.getLibrary().indigoGetLastError());
        }

        return result;
    }

	/**
			Creates a chemical storage of a specified type in a specified location

		@param indigo Indigo instance
		@param location Directory with the files location
		@param type molecule" or "reaction"
		@param options additional options separated with a semicolon. See the Bingo documentation for more detail
		@return Bingo database instance
	*/
	public static Bingo createDatabaseFile(Indigo indigo, String location, String type, String options) {
	    indigo.setSessionID();
	    if (options == null)
	    {
	        options = "";
	    }
	    return new Bingo(indigo, location, type, options);
	}

	/**
		Creates a chemical storage of a specified type in a specified location

		@param indigo Indigo instance
		@param location Directory with the files location
		@param type molecule" or "reaction"
		@return Bingo database instance
	*/
	public static Bingo createDatabaseFile(Indigo indigo, String location, String type) {
		return createDatabaseFile(indigo, location, type, null);
	}

	/**
		Loads a chemical storage of a specified type from a specified location

		@param indigo Indigo instance
		@param location Directory with the files location
	 	@param options Additional options separated with a semicolon. See the Bingo documentation for more details
	 	@return Bingo database instance
	*/
	public static Bingo loadDatabaseFile(Indigo indigo, String location, String options) {
		if (options == null) {
			options = "";
		}
		return new Bingo(indigo, location, options);
	}

	/**
		Loads a chemical storage of a specified type from a specified location

		@param indigo Indigo instance
		@param location Directory with the files location
	 	@return Bingo database instance
	*/
	public static Bingo loadDatabaseFile(Indigo indigo, String location) {
		return loadDatabaseFile(indigo, location, null);
	}

    /**
       	Insert a structure into the database and returns id of this structure

        @param record Indigo object with a chemical structure (molecule or reaction)
        @return record id
    */
	public int insert(IndigoObject record) {
	   _indigo.setSessionID();
	   return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObj(_id, record.self));
	}

    /**
        Inserts a structure under a specified id

        @param record Indigo object with a chemical structure (molecule or reaction)
        @param id record id
        @return inserted record id
    */
	public int insert(IndigoObject record, int id) {
		_indigo.setSessionID();
		return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithId(_id, record.self, id));
	}

    /**
       	Insert a structure into the database and returns id of this structure

        @param record Indigo object with a chemical structure (molecule or reaction)
        @param ext_fp Indigo object with a external similarity fingerprint (molecule or reaction)
        @return record id
    */
	public int insertWithExtFP(IndigoObject record, IndigoObject ext_fp) {
	   _indigo.setSessionID();
	   return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithExtFP(_id, record.self, ext_fp.self));
	}

    /**
        Inserts a structure under a specified id

        @param record Indigo object with a chemical structure (molecule or reaction)
        @param id record id
        @param ext_fp Indigo object with a external similarity fingerprint (molecule or reaction)
        @return inserted record id
    */
	public int insertWithExtFP(IndigoObject record, IndigoObject ext_fp, int id) {
		_indigo.setSessionID();
		return Bingo.checkResult(_indigo, _lib.bingoInsertRecordObjWithIdAndExtFP(_id, record.self, id, ext_fp.self));
	}

	/**
        Delete a record by id

        @param id Record id
    */
	public void delete(int id) {
		_indigo.setSessionID();
		Bingo.checkResult(_indigo, _lib.bingoDeleteRecord(_id, id));
	}

	/**
		Execute substructure search operation

		@param query Indigo query object (molecule or reaction)
		@param options Search options
		@return Bingo search object instance
	*/
	public BingoObject searchSub(IndigoObject query, String options) {
		if (options == null) {
			options = "";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSub(_id, query.self, options)), _indigo, _lib);
	}

	/**
		Execute substructure search operation

		@param query Indigo query object (molecule or reaction)
		@return Bingo search object instance
	*/
    public BingoObject searchSub(IndigoObject query) {
        return searchSub(query, null);
    }

	/**
		Execute similarity search operation

		@param query indigo object (molecule or reaction)
		@param min Minimum similarity value
		@param max Maximum similarity value
		@param metric Default value is "tanimoto"
		@return Bingo search object instance
	*/
	public BingoObject searchSim(IndigoObject query, float min, float max, String metric) {
		if (metric == null) {
			metric = "tanimoto";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSim(_id, query.self, min, max, metric)), _indigo, _lib);
	}


	/**
		Execute  similarity search operation

		@param query indigo object (molecule or reaction)
		@param min Minimum similarity value
		@param max Maximum similarity value
		@return Bingo search object instance
	*/
        public BingoObject searchSim(IndigoObject query, float min, float max) {
		return searchSim(query, min, max, null);
	}
	/**
		Execute similarity search with external fingerprint

		@param query indigo object (molecule or reaction)
		@param min Minimum similarity value
		@param max Maximum similarity value
		@param ext_fp Indigo object with a external similarity fingerprint (molecule or reaction)
		@param metric Default value is "tanimoto"
		@return Bingo search object instance
	*/
	public BingoObject searchSimWithExtFP(IndigoObject query, float min, float max, IndigoObject ext_fp, String metric) {
		if (metric == null) {
			metric = "tanimoto";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSimWithExtFP(_id, query.self, min, max, ext_fp.self, metric)), _indigo, _lib);
	}
        /**
		Execute similarity search with external fingerprint

		@param query indigo object (molecule or reaction)
		@param min Minimum similarity value
		@param max Maximum similarity value
		@param ext_fp Indigo object with a external similarity fingerprint (molecule or reaction)
		@return Bingo search object instance
	*/
	public BingoObject searchSimWithExtFP(IndigoObject query, float min, float max, IndigoObject ext_fp) {
		return searchSimWithExtFP(query, min, max, ext_fp, null);
	}
        
        /**
		Execute similarity search for most similar structures (defined by limit)

		@param query indigo object (molecule or reaction)
		@param limit Number of structures
		@param minSim Minimum similarity value
		@param metric Default value is "tanimoto"
		@return Bingo search object instance
	*/
	public BingoObject searchSimTopN(IndigoObject query, int limit, float minSim, String metric) {
		if (metric == null) {
			metric = "tanimoto";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSimTopN(_id, query.self, limit, minSim, metric)), _indigo, _lib);
	}
        /**
		Execute similarity search for most similar structures (defined by limit)

		@param query indigo object (molecule or reaction)
		@param limit Number of structures
		@param minSim Minimum similarity value
		@return Bingo search object instance
	*/
	public BingoObject searchSimTopN(IndigoObject query, int limit, float minSim) {
		return searchSimTopN(query, limit, minSim, null);
	}
        
        /**
		Execute similarity search for most similar structures with external fingerprint

		@param query indigo object (molecule or reaction)
		@param limit Number of structures
		@param minSim Minimum similarity value
		@param metric Default value is "tanimoto"
                @param extFp Indigo object with a external similarity fingerprint (molecule or reaction)
		@return Bingo search object instance
	*/
	public BingoObject searchSimTopNWithExtFP(IndigoObject query, int limit, float minSim, IndigoObject extFp, String metric) {
		if (metric == null) {
			metric = "tanimoto";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchSimTopNWithExtFP(_id, query.self, limit, minSim, extFp.self, metric)), _indigo, _lib);
	}
        
        /**
		Execute similarity search for most similar structures with external fingerprint

		@param query indigo object (molecule or reaction)
		@param limit Number of structures
		@param minSim Minimum similarity value
                @param extFp Indigo object with a external similarity fingerprint (molecule or reaction)
		@return Bingo search object instance
	*/
	public BingoObject searchSimTopNWithExtFP(IndigoObject query, int limit, float minSim, IndigoObject extFp) {
		return searchSimTopNWithExtFP(query, limit, minSim, extFp, null);
	}
        
        
        

	/**
		Execute enumerate id operation

		@return Bingo enumerate id object instance
	*/
	public BingoObject enumerateId() {
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoEnumerateId(_id)), _indigo, _lib);
	}

    /**
    	Perform exact search operation

    	@param query indigo object (molecule or reaction)
    	@param options search options
    	@return Bingo search object instance
    */
	public BingoObject searchExact(IndigoObject query, String options) {
		if (options == null) {
			options = "";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchExact(_id, query.self, options)), _indigo, _lib);
	}

    /**
    	Perform exact search operation

    	@param query indigo object (molecule or reaction)
    	@return Bingo search object instance
    */
	public BingoObject searchExact(IndigoObject query) {
		return searchExact(query, null);
	}

    /**
        Perform search by molecular formula

        @param query string with formula to search. For example, "C22 H23 N3 O2"
        @param options search options
        @return Bingo search object instance
    */
	public BingoObject searchMolFormula(String query, String options) {
		if (options == null) {
			options = "";
		}
		_indigo.setSessionID();
		return new BingoObject(Bingo.checkResult(_indigo, _lib.bingoSearchMolFormula(_id, query, options)), _indigo, _lib);
	}

    /**
        Perform search by molecular formula

        @param query string with formula to search. For example, "C22 H23 N3 O2"
        @return Bingo search object instance
    */
	public BingoObject searchMolFormula(String query) {
		return searchMolFormula(query, null);
	}

   	/**
        Post-process index optimization
    */
	public void optimize () {
		_indigo.setSessionID();
		Bingo.checkResult(_indigo, _lib.bingoOptimize(_id));
	}

	/**
        Returns an IndigoObject for the record with the specified id

        @param id record id
		@return Indigo object
    */
	public IndigoObject getRecordById(int id) {
		_indigo.setSessionID();
		return new IndigoObject(_indigo, Bingo.checkResult(_indigo, _lib.bingoGetRecordObj(_id, id)));
	}



    /**
     Returns Bingo version

     @return Bingo version
     */
    public String version() {
        _indigo.setSessionID();
        return Bingo.checkResult(_indigo, _lib.bingoVersion());
    }
}