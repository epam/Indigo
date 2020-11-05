package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public final class TverskySimilarityMatch<T extends IndigoRecord> extends BaseMatch<T> {

    private final float alpha;
    private final float beta;

    public TverskySimilarityMatch(T target, float threshold, float alpha, float beta) {
        super(target, threshold);
        this.alpha = alpha;
        this.beta = beta;
    }

    public TverskySimilarityMatch(T target, float alpha, float beta) {
        super(target);
        this.alpha = alpha;
        this.beta = beta;
    }

    public TverskySimilarityMatch(T target) {
        super(target);
        this.alpha = 0.5f;
        this.beta = 0.5f;
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / ((params.a - _score) * params.alpha + (doc['" + NamingConstants.SIM_FINGERPRINT_LEN + "'].value - _score) * params.beta + _score)");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getSimFingerprint().size());
        params.put("alpha", this.alpha);
        params.put("beta", this.beta);
        map.put("params", params);
        return Script.parse(map);
    }

    @Override
    public String getMinimumShouldMatch(int length) {
        double top = this.alpha * getTarget().getSimFingerprint().size() + this.beta;
        double down = getThreshold() + this.alpha + this.beta - 1.0f;
        double mm = Math.floor((top / down)) / length;
        return (int) (mm * 100) + "%";
    }

    @Override
    public String getFingerprintName() {
        return NamingConstants.SIM_FINGERPRINT;
    }
}
