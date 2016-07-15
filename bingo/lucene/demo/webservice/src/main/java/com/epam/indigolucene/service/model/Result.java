package com.epam.indigolucene.service.model;

/**
 * Created by Artem Malykh on 20.06.16.
 */
public class Result<T> {
    public static enum Status {
        OK,
        ERROR
    }

    T payload;
    String message;
    Status status;

    public Result(T payload) {
        this.payload = payload;
    }

    public Result(String message) {
        this.message = message;
    }

    public T getPayload() {
        return payload;
    }

    public void setPayload(T payload) {
        this.payload = payload;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Status getStatus() {
        return status;
    }

    public void setStatus(Status status) {
        this.status = status;
    }

    public static <T> Result<T> success(T payload) {
        Result<T> res = new Result<T>(payload);
        res.setStatus(Status.OK);
        return res;

    }

    public static <T> Result<T> error(String msg) {
        Result<T> res = new Result<T>(msg);
        res.setStatus(Status.ERROR);
        return res;

    }
}
