using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.ggasoftware.indigo
{
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
	public unsafe class Bingo
	{
		private Indigo _indigo;
		private BingoLib _lib;
		private int _id;

		private Bingo (Indigo indigo, int id, BingoLib lib)
		{
			_indigo = indigo;
			_lib = lib;
			_id = id;
		}

		~Bingo()
		{
			_indigo.checkResult(_lib.bingoCloseDatabase(_id));
			_id = -1;
		}

		private static BingoLib getLib(Indigo indigo)
		{
			String dllpath = indigo.getDllPath();
			string libraryName;
			IndigoDllLoader dll_loader = IndigoDllLoader.Instance;
			switch (Environment.OSVersion.Platform)
			{
			case PlatformID.Win32NT:
				libraryName = "bingo.dll";
				dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesWin", false);
				break;
			case PlatformID.Unix:
				if (IndigoDllLoader.isMac())
				{
					libraryName = "libbingo.dylib";
					dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesMac", false);
				}
				else
				{
					libraryName = "libbingo.so";
					dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesLinux", false);
				}
				break;
			default:
				throw new PlatformNotSupportedException (String.Format ("Unsupported platform: {0}", Environment.OSVersion.Platform));
			}
			
			return dll_loader.getInterface<BingoLib>(libraryName);
		}

		public static Bingo createDatabaseFile(Indigo indigo, string location, string type, string options)
		{
			if (options == null)
			{
				options = "";
			}
			BingoLib lib = Bingo.getLib(indigo);
			return new Bingo(indigo, indigo.checkResult(lib.bingoCreateDatabaseFile(location, type, options)), lib);
		}

		public static Bingo loadDatabaseFile(Indigo indigo, string location, string type)
		{
			BingoLib lib = Bingo.getLib(indigo);
			return new Bingo(indigo, indigo.checkResult(lib.bingoLoadDatabaseFile(location, type)), lib);
		}


		public void insert(IndigoObject record)
		{
			_indigo.checkResult(_lib.bingoInsertRecordObj(_id, record.self));
		}

		public void delete(int index)
		{
			_indigo.checkResult(_lib.bingoDeleteRecord(_id, index));
		}

		public BingoObject searchSub(IndigoObject query, string options)
		{
			if (options == null)
			{
				options = "";
			}
			return new BingoObject(_indigo.checkResult(_lib.bingoSearchSub(_id, query.self, options)), _indigo, _lib);
		}

		public BingoObject searchSim(IndigoObject query, float min, float max, string metric)
		{
			if (metric == null)
			{
				metric = "tanimoto";
			}
			return new BingoObject(_indigo.checkResult(_lib.bingoSearchSim(_id, query.self, min, max, metric)), _indigo, _lib);
		}
	}
}
