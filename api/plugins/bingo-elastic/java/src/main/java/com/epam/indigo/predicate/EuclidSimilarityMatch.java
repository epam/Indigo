package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public class EuclidSimilarityMatch<T extends IndigoRecord> extends SimilarityMatch<T> {

    public EuclidSimilarityMatch(T target, float threshold) {
        super(target, threshold);
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / params.a");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getFingerprint().size());
        map.put("params", params);
        return Script.parse(map);
    }

    public EuclidSimilarityMatch(T target) {
        super(target);
    }
}
