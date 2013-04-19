using System;
using System.Collections;

namespace com.ggasoftware.indigo
{
	public class BingoObject : IEnumerable
	{
		public int self;
		private Indigo _indigo;
		private BingoLib _bingo_lib;

		public BingoObject(int self, Indigo indigo, BingoLib bingo_lib)
		{
			this.self = self;
			this._indigo = indigo;
			this._bingo_lib = bingo_lib;
		}

		public void next()
		{
			_indigo.setSessionID();
			self = _indigo.checkResult(_bingo_lib.bingoNext(self));
		}

		public int getCurrentIndex()
		{
			_indigo.setSessionID();
			return _indigo.checkResult(_bingo_lib.bingoGetCurrentIndex(self));
		}

		public IndigoObject getIndigoObject()
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
