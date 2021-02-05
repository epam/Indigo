package com.epam.indigo.model;

import com.epam.indigo.IndigoException;

public interface ErrorHandler {
    void handle(IndigoException error);
}
