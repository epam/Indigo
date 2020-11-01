package com.epam.indigo.predicate;

import com.epam.indigo.Indigo;
import com.epam.indigo.model.IndigoRecord;

import java.util.function.Predicate;

public class SubstructureMatch<T extends IndigoRecord> extends ExactMatch<T> {

    public SubstructureMatch(T target) {
        super(target);
    }

    public static Predicate<IndigoRecord> substructureMatchAfterChecker(IndigoRecord target, Indigo indigo) {
        return candidate -> indigo
                .substructureMatcher(candidate.getIndigoObject(indigo))
                .match(indigo.loadQueryMolecule(target.getIndigoObject(indigo).canonicalSmiles())) != null;
    }
}
