package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

/**
 * Similarity match based on Tanimoto metric
 * @see <a href="https://en.wikipedia.org/wiki/Jaccard_index#Tanimoto_similarity_and_distance">Tanimoto_similarity_and_distance</a>
 */
public final class SimilarityMatch<T extends IndigoRecord> extends BaseMatch<T> {

    public SimilarityMatch(T target, float threshold) {
        super(target, threshold);
    }

    public SimilarityMatch(T target) {
        super(target);
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / (params.a + doc['" + NamingConstants.SIM_FINGERPRINT_LEN + "'].value - _score)");
        Map<String, Object> params = new HashMap<>();
        params.put("a", getTarget().getSimFingerprint().size());
        map.put("params", params);
        return Script.parse(map);
    }

    @Override
    public String getMinimumShouldMatch(int length) {
        double mm = Math.floor((getThreshold() * (getTarget().getSimFingerprint().size() + 1)) / (1.0f + getThreshold())) / length;
        return (int) (mm * 100) + "%";
    }

    @Override
    public String getFingerprintName() {
        return NamingConstants.SIM_FINGERPRINT;
    }
}
