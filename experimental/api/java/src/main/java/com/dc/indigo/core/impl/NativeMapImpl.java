package com.dc.indigo.core.impl;


public class NativeMapImpl<K, V> extends com.dc.indigo.core.impl.ItemImpl implements com.dc.indigo.core.NativeMap<K, V> {
	
	protected NativeMapImpl(int thisID) {
		super(thisID);
	}
 
	@Override
	public V remove(Object key) {
		// TODO Auto-generated method stub
		return null;
	}
    
	private native int _remove(int thisID, java.lang.Object key); //
    
	@Override
	public V get(Object key) {
		// TODO Auto-generated method stub
		return null;
	}
    
	private native int _get(int thisID, java.lang.Object key); //
    
	public java.lang.Object put(java.lang.Object key, java.lang.Object value) {
		return	_put(thisID, key, value);
	}
    
	private native java.lang.Object _put(int thisID, java.lang.Object key, java.lang.Object value); //
    
	public java.util.Collection<V> values() {
		return null; //	session.getCollection<V>(_values(thisID));
	}
    
	private native int _values(int thisID); //
    
	public void clear() {
			_clear(thisID);
	}
    
	private native void _clear(int thisID); //
    
	public int size() {
		return	_size(thisID);
	}
    
	private native int _size(int thisID); //
    
	public java.util.Set<java.util.Map.Entry<K, V>> entrySet() {
		return	null; //session.getEntrySet(_entrySet(thisID));
	}
    
	private native int _entrySet(int thisID); //
    
	public boolean containsKey(java.lang.Object key) {
		return	_containsKey(thisID, key);
	}
    
	private native boolean _containsKey(int thisID, java.lang.Object key); //
    
	public java.util.Set<K> keySet() {
		return	null; //session.getSet(_keySet(thisID));
	}
    
	private native int _keySet(int thisID); //
    
	public boolean containsValue(java.lang.Object value) {
		return	_containsValue(thisID, value);
	}
    
	private native boolean _containsValue(int thisID, java.lang.Object value); //

}



