using System;
using System.Collections;

namespace com.ggasoftware.indigo
{
	public class BingoObject
	{
		private int _self;
		private Indigo _indigo;
		private BingoLib _bingo_lib;

		public BingoObject(int self, Indigo indigo, BingoLib bingo_lib)
		{
			this.self = self;
			this._indigo = indigo;
			this._bingo_lib = bingo_lib;
		}

        public int self
        {
            get { return _self; }
            set { _self = value; }
        }

		public bool next()
		{
			_indigo.setSessionID();
			return (_indigo.checkResult(_bingo_lib.bingoNext(self)) == 1) ? true : false;
		}

		public int getCurrentId()
		{
			_indigo.setSessionID();
			return _indigo.checkResult(_bingo_lib.bingoGetCurrentId(self));
		}

		public IndigoObject getObject()
		{
			_indigo.setSessionID();
			return new IndigoObject(_indigo, _indigo.checkResult(_bingo_lib.bingoGetObject(self)));
		}

		public void endSearch()
		{
			_indigo.setSessionID();
			_indigo.checkResult(_bingo_lib.bingoEndSearch(self));
		}
	}
}
