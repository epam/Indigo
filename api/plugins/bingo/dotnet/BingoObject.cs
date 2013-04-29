using System;
using System.Collections;

namespace com.ggasoftware.indigo
{
	public class BingoObject
	{
		private int _id;
		private Indigo _indigo;
		private BingoLib _bingoLib;

		public BingoObject(int id, Indigo indigo, BingoLib bingo_lib)
		{
			this._id = id;
			this._indigo = indigo;
			this._bingoLib = bingo_lib;
		}

		~BingoObject()
		{
			Bingo.checkResult(_indigo, _bingoLib.bingoEndSearch(_id));
			_id = -1;
		}

        public int id
        {
            get { return _id; }
            set { _id = value; }
        }

		public bool next()
		{
			return (Bingo.checkResult(_indigo, _bingoLib.bingoNext(_id)) == 1) ? true : false;
		}

		public int getCurrentId()
		{
			return Bingo.checkResult(_indigo, _bingoLib.bingoGetCurrentId(_id));
		}

		public IndigoObject getIndigoObject()
		{
			return new IndigoObject(_indigo, Bingo.checkResult(_indigo, _bingoLib.bingoGetObject(_id)));
		}
	}
}
