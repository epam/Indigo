package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public class ExactMatch<T extends IndigoRecord> extends SimilarityMatch<T> {

    public ExactMatch(T target) {
        super(target);
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
}
