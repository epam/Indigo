package com.ggasoftware.indigo;

import com.sun.jna.ptr.FloatByReference;

/**
	Bingo search object
*/
public class BingoObject {
	private int _id;
	private Indigo _indigo;
	private BingoLib _bingoLib;
    private Object _reference;

	BingoObject(int id, Indigo indigo, BingoLib bingo_lib) {
		_id = id;
		_indigo = indigo;
		_bingoLib = bingo_lib;
	}

	protected void finalize() {
		dispose();
	}

	public void dispose() {
        if (_id >= 0) {
	       Bingo.checkResult(_indigo, _bingoLib.bingoEndSearch(_id));
            _id = -1;
        }
	}

	public void close() {
		dispose();
	}


	/**
        Move to the next record

        return True if there are any more records
    */
    public boolean next() {
        _indigo.setSessionID();
        return (_bingoLib.bingoNext(_id) == 1);
	}

	/**
	   Return current record id. Should be called after next() method.

	   @return Record id
    */
	public int getCurrentId() {
        _indigo.setSessionID();
        return Bingo.checkResult(_indigo, _bingoLib.bingoGetCurrentId(_id));
	}

    /**
        Return current similarity value. Should be called after next() method.

        @return Similarity value
    */
	public float getCurrentSimilarityValue()
	{
        _indigo.setSessionID();
        return Bingo.checkResult(_indigo, _bingoLib.bingoGetCurrentSimilarityValue(_id));
	}

    /**
	   Return a shared IndigoObject for the matched target

	   @return Shared Indigo object for the current search operation
    **/
	public IndigoObject getIndigoObject() {
        _indigo.setSessionID();
        IndigoObject res = new IndigoObject(_indigo, Bingo.checkResult(_indigo, _bingoLib.bingoGetObject(_id)));
        _reference = res;
        return res;
	}

	/**
	   Estimate remaining hits count

	   @return Estimated hits count
    */
	public int estimateRemainingResultsCount() {
        _indigo.setSessionID();
        return Bingo.checkResult(_indigo, _bingoLib.bingoEstimateRemainingResultsCount(_id));
	}

	/**
        Estimate remaining hits count error

	   @return Estimated hits count error
    */
	public int estimateRemainingResultsCountError() {
        _indigo.setSessionID();
        return Bingo.checkResult(_indigo, _bingoLib.bingoEstimateRemainingResultsCountError(_id));
	}

    /**
        Estimate remaining search time

       @return Estimated remaining search time
    */
	public float estimateRemainingTime () {
        FloatByReference estimated_time = new FloatByReference();
        _indigo.setSessionID();
        Bingo.checkResult(_indigo, _bingoLib.bingoEstimateRemainingTime(_id, estimated_time));
        return estimated_time.getValue();
	}
}
