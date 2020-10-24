package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public class TanimotoSimilarityMatch<T extends IndigoRecord> extends SimilarityMatch<T> {

    public TanimotoSimilarityMatch(T target, float threshold) {
        super(target, threshold);
    }

    public TanimotoSimilarityMatch(T target) {
        super(target);
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / (params.a + doc['fingerprint_len'].value - _score)");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getFingerprint().size());
        map.put("params", params);
        return Script.parse(map);
    }
}
