package com.epam.indigo;

import com.sun.jna.ptr.FloatByReference;

/**
 * Bingo search object
 */
public class BingoObject {
    private int id;
    private final Indigo indigo;
    private final BingoLib bingoLib;
    private Object reference;

    BingoObject(int id, Indigo indigo, BingoLib bingo_lib) {
        this.id = id;
        this.indigo = indigo;
        bingoLib = bingo_lib;
    }

    protected void finalize() {
        dispose();
    }

    public void dispose() {
        if (id >= 0) {
            Bingo.checkResult(indigo, bingoLib.bingoEndSearch(id));
            id = -1;
        }
    }

    public void close() {
        dispose();
    }


    /**
     * Move to the next record
     * <p>
     * return True if there are any more records
     */
    public boolean next() {
        indigo.setSessionID();
        return (bingoLib.bingoNext(id) == 1);
    }

    /**
     * Return current record id. Should be called after next() method.
     *
     * @return Record id
     */
    public int getCurrentId() {
        indigo.setSessionID();
        return Bingo.checkResult(indigo, bingoLib.bingoGetCurrentId(id));
    }

    /**
     * Return current similarity value. Should be called after next() method.
     *
     * @return Similarity value
     */
    public float getCurrentSimilarityValue() {
        indigo.setSessionID();
        return Bingo.checkResult(indigo, bingoLib.bingoGetCurrentSimilarityValue(id));
    }

    /**
     * Return a shared IndigoObject for the matched target
     *
     * @return Shared Indigo object for the current search operation
     **/
    public IndigoObject getIndigoObject() {
        indigo.setSessionID();
        IndigoObject res = new IndigoObject(indigo, Bingo.checkResult(indigo, bingoLib.bingoGetObject(id)));
        reference = res;
        return res;
    }

    /**
     * Estimate remaining hits count
     *
     * @return Estimated hits count
     */
    public int estimateRemainingResultsCount() {
        indigo.setSessionID();
        return Bingo.checkResult(indigo, bingoLib.bingoEstimateRemainingResultsCount(id));
    }

    /**
     * Estimate remaining hits count error
     *
     * @return Estimated hits count error
     */
    public int estimateRemainingResultsCountError() {
        indigo.setSessionID();
        return Bingo.checkResult(indigo, bingoLib.bingoEstimateRemainingResultsCountError(id));
    }

    /**
     * Estimate remaining search time
     *
     * @return Estimated remaining search time
     */
    public float estimateRemainingTime() {
        FloatByReference estimated_time = new FloatByReference();
        indigo.setSessionID();
        Bingo.checkResult(indigo, bingoLib.bingoEstimateRemainingTime(id, estimated_time));
        return estimated_time.getValue();
    }
}
