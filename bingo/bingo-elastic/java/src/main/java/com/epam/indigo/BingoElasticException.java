package com.epam.indigo;

public class BingoElasticException extends RuntimeException {

    public BingoElasticException(String message) {
        super(message);
    }

    public BingoElasticException(String message, Throwable cause) {
        super(message, cause);
    }
}
