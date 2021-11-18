package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public final class EuclidSimilarityMatch<T extends IndigoRecord> extends BaseMatch<T> {

    public EuclidSimilarityMatch(T target, double threshold) {
        super(target, threshold);
    }

    public EuclidSimilarityMatch(T target) {
        super(target);
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / params.a");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getSimFingerprint().size());
        map.put("params", params);
        return Script.parse(map);
    }

    @Override
    public String getMinimumShouldMatch(int length) {
        double mm = Math.floor(getThreshold() * getTarget().getSimFingerprint().size()) / length;
        return (int) (mm * 100) + "%";
    }

    @Override
    public String getFingerprintName() {
        return NamingConstants.SIM_FINGERPRINT;
    }
}
