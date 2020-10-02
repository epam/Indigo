package com.epam.indigo;

import java.util.function.Predicate;

public class IndigoQuery<T extends IndigoRecord> implements Predicate<T> {


    @Override
    public boolean test(T t) {
        return false;
    }

    @Override
    public Predicate<T> and(Predicate<? super T> other) {
        return null;
    }

    @Override
    public Predicate<T> negate() {
        return null;
    }

    @Override
    public Predicate<T> or(Predicate<? super T> other) {
        return null;
    }

//     similar
//    molecules.filter(t -> t.getMoleculeWeight() > 10)

//                .search(t -> t.similar("H2S04")

}
