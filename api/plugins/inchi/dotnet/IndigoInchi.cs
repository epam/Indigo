using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace com.ggasoftware.indigo
{
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
	public unsafe class IndigoInchi
	{
		private Indigo _indigo;
		private IndigoInchiLib _inchi_lib;

		public IndigoInchi (Indigo indigo)
		{
			String dllpath = indigo.getDllPath ();
			string libraryName;
			IndigoDllLoader dll_loader = IndigoDllLoader.Instance;
			switch (Environment.OSVersion.Platform) {
			case PlatformID.Win32NT:
				libraryName = "indigo-inchi.dll";
				dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesWin", false);
				break;
			case PlatformID.Unix:
				if (IndigoDllLoader.isMac()) {
					libraryName = "libindigo-inchi.dylib";
					dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesMac", false);
				} else {
					libraryName = "libindigo-inchi.so";
					dll_loader.loadLibrary (dllpath, libraryName, "com.ggasoftware.indigo.Properties.ResourcesLinux", false);
				}
				break;
			default:
				throw new PlatformNotSupportedException (String.Format ("Unsupported platform: {0}", Environment.OSVersion.Platform));
			}
			
			_inchi_lib = dll_loader.getInterface<IndigoInchiLib> (libraryName);

			_indigo = indigo;
		}

		public String version ()
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiVersion ()));
		}
		
		public int resetOptions ()
		{
			_indigo.setSessionID ();
			return _indigo.checkResult (_inchi_lib.indigoInchiResetOptions ());
		}
		
		public IndigoObject loadMolecule (String inchi_string)
		{
			_indigo.setSessionID ();
			return new IndigoObject (_indigo, _indigo.checkResult (_inchi_lib.indigoInchiLoadMolecule (inchi_string)));
		}
		
		public String getInchi (IndigoObject molecule)
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiGetInchi (molecule.self)));
		}
		
		public String getInchiKey (String inchi_string)
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiGetInchiKey (inchi_string)));
		}
		
		public String getWarning ()
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiGetWarning ()));
		}
		
		public String getLog ()
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiGetLog ()));
		}
		
		public String getAuxInfo ()
		{
			_indigo.setSessionID ();
			return new String (_indigo.checkResult (_inchi_lib.indigoInchiGetAuxInfo ()));
		}
	}
}
