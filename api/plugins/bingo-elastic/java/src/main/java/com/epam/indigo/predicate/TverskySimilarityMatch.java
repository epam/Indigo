package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public class TverskySimilarityMatch<T extends IndigoRecord> extends SimilarityMatch<T> {

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

    //    tversky <alpha> <beta>: c / ((a - c) * alpha + (b - c) * beta)
    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / ((params.a - _score) * params.alpha + (doc['fingerprint_len'].value - _score) * params.beta + _score)");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getFingerprint().size());
        params.put("alpha", this.alpha);
        params.put("beta", this.beta);
        map.put("params", params);
        return Script.parse(map);
    }
}
