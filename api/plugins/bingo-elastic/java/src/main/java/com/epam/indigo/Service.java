package com.epam.indigo;

//import static java.util.function.Predicate.not;

public class Service<T extends IndigoRecord> {

    private final ElasticRepository<T> elasticService;

    public Service(String hostname, int port, String scheme) {
        elasticService = new ElasticRepository<>(hostname, port, scheme);
    }
}

//    public Stream<T> stream(int size, int batchSize, IndigoQuery indigoQuery) {
//        final Cursor cursor = new Cursor();
//        return Stream
//                .generate(() -> next(cursor, size, batchSize))
//                .takeWhile(not(List::isEmpty))
//                .flatMap(List::stream);
////        return Stream
////                .generate(() -> next(cursor, size, batchSize))
////                .takeWhile(not(List::isEmpty))
////                .flatMap(List::stream);
//    }
//
//    private static <T> T next(Cursor cursor, int size, int batchSize) {
//    }

//    private <T> List<T> next(Cursor cursor, int size, int batchSize) {
//        int fetchSize = Math.min(size - cursor.offset, batchSize);
//        int result = elasticService.fetch(cursor.offset, fetchSize);
//        cursor.inc(result.size());
//        return result;
//    }

//    public List<T> next(Cursor cursor, int size, int batchSize) {
//        int fetchSize = Math.min(size - cursor.offset, batchSize);
//        int result = elasticService.fetch(cursor.offset, fetchSize);
//        cursor.inc(result.size());
//        return result;
//    }

    /*private static class Cursor {

        private int offset;

        void inc(int by) {
            offset += by;
        }
    }*/
//}
