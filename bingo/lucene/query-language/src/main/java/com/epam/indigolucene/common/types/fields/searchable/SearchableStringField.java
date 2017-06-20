package com.epam.indigolucene.common.types.fields.searchable;

import com.epam.indigolucene.common.types.conditions.stringconditions.StringInCondition;
import com.epam.indigolucene.common.types.conditions.stringconditions.StringStartsWithCondition;
import com.epam.indigolucene.common.types.fields.StringField;

import java.util.Collection;
/**
 * This class is a type of all searchable string fields of Solr's schema.xml. All string manipulation methods are
 * returned from here.
 *
 * @author Artem Malykh
 * created on 2016-03-30
 */
public class SearchableStringField<S> extends StringField<S> {
    public SearchableStringField(String name, boolean isMultiple) {
        super(name, isMultiple);
    }

    public StringStartsWithCondition<S> startsWith(String starter) {
        return new StringStartsWithCondition<>(this, starter);
    }

    public StringInCondition<S> in(Collection<String> strings) {
        return new StringInCondition<>(this, strings);
    }
}
