package com.dc.indigo.core;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

import com.dc.builder.annotations.Ignore;

@Ignore
public interface NativeMap<K, V> extends Item, Map<K, V> {

	@Override
	public int size();

	@Override
	public default boolean isEmpty() {
		return size() == 0;
	}

	@Override
	public boolean containsKey(Object key);

	@Override
	public boolean containsValue(Object value);

	@Override
	public V get(Object key);

	@Override
	public V put(K key, V value);

	@Override
	public V remove(Object key);

	@Override
	public default void putAll(Map<? extends K, ? extends V> m) {
		m.entrySet().stream().forEach(e -> put(e.getKey(), e.getValue()));
	}

	@Override
	public void clear();

	@Override
	public Set<K> keySet();

	@Override
	public Collection<V> values();

	@Override
	public Set<Entry<K, V>> entrySet();

}
