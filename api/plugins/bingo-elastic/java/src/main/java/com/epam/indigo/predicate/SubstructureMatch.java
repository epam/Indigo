package com.epam.indigo.predicate;

import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.NamingConstants;
import org.elasticsearch.script.Script;

import java.util.HashMap;
import java.util.Map;

public class SubstructureMatch<T extends IndigoRecord> extends ExactMatch<T> {

    public SubstructureMatch(T target) {
        super(target);
    }

    @Override
    public String getFingerprintName() {
        return NamingConstants.SUB_FINGERPRINT;
    }

    @Override
    public Script generateScript() {
        Map<String, Object> map = new HashMap<>();
        map.put("source", "_score / doc['" + NamingConstants.SUB_FINGERPRINT_LEN + "'].value");
        return Script.parse(map);
    }

    @Override
    public String getMinimumShouldMatch(int length) {
        return "100%";
    }
}
