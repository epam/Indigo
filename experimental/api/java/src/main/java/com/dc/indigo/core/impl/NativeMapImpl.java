package com.dc.indigo.core.impl;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

public class NativeMapImpl<K,V> implements Map<K, V> {

	@Override
	public native int size();

	@Override
	public boolean isEmpty() {
		return size() == 0;
	}

	@Override
	public native boolean containsKey(Object key);

	@Override
	public native boolean containsValue(Object value);

	@Override
	public V get(Object key) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public V put(K key, V value) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public V remove(Object key) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void putAll(Map<? extends K, ? extends V> m) {
		// TODO Auto-generated method stub

	}

	@Override
	public native void clear();
	@Override
	public Set<K> keySet() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Collection<V> values() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<Entry<K, V>> entrySet() {
		// TODO Auto-generated method stub
		return null;
	}

}
