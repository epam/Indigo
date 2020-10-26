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
        map.put("source", "_score / doc['fingerprint_len'].value");
        return Script.parse(map);
    }

    @Override
    public String getMinimumShouldMatch(int length) {
        return "100%";
    }

    @Override
    public float getThreshold() {
        return 1.0f;
    }
}
