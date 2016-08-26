/****************************************************************************
* Copyright (C) 2009-2016 EPAM Systems
*
* This file is part of Indigo toolkit.
*
* This file may be distributed and/or modified under the terms of the
* GNU General Public License version 3 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in the
* packaging of this file.
*
* This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
* WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
***************************************************************************/

#ifndef __trie_h__
#define __trie_h__

#include <map>
#include <set>
#include <string>

#include "non_copyable.h"

namespace indigo {

	// Forward declaration
	template <typename T> class Trie;

	// Type alias for (sub)trie
	template <typename T> using Leaves = std::map<char, Trie<T>*>;

	// A collection of words that a (sub)trie holds
	typedef std::set<std::string> Wordset;

	/* 
	 A trie (https://en.wikipedia.org/wiki/Trie)
	 Also holds an arbitrary data type, associated with a given trie
	 Currently, doesn't provide a delete/remove operation
	 */
	template <typename T>
	class Trie : public NonCopyable {

		T		  _data;						// A dataset associated with a given word
		Leaves<T> _leaves;						// (Sub)trie(s)
		bool	  _mark;						// A terminator flag

		void getWords(Wordset& words, std::string& buffer) const;
		void getWordsWithPrefix(const std::string& prefix, Wordset& words, std::string& buffer) const;

	public:
		inline explicit Trie(bool mark = false) : _mark{ mark } { }
		virtual ~Trie();

		inline T& getData() { return _data; }
		inline const T& getData() const { return _data; }
		inline void setData(const T& data) { _data = data; }

		inline bool isMark() const { return _mark; }

		// Adds a word into the trie, setting up data in the end
		void addWord(const std::string& word, const T& data);

		// Retrieves a list of words that a given (sub)trie holds
		void getWords(Wordset& words) const;

		// Retrievs a list of words which start from a given prefix
		void getWordsWithPrefix(const std::string& prefix, Wordset& words) const;

		/*
		 Returns a terminating node for a given word,
		 nullptr if there's no such word in a trie
		 */
		const Trie* getNode(const std::string& word) const;

		/*
		Returns true if trie contains the word
		*/
		bool isWord(const std::string& word) const;
	}; // class Trie

	template <typename T>
	Trie<T>::~Trie() {
		for (auto& it : _leaves)
			delete it.second;
	}

	// Adds a word into the trie, setting up data in the end
	template <typename T>
	void Trie<T>::addWord(const std::string& word, const T& data) {
		if (!word.empty()) {
			const std::string& remainder = word.substr(1);
			const char ch = word[0];

			if (_leaves[ch]) {
				if (word.length() == 1)
					_leaves[ch]->_mark = true;
				else
					_leaves[ch]->addWord(remainder, data);
			} else {
				Trie* trie = new Trie(word.length() == 1);
				trie->addWord(remainder, data);
				_leaves[ch] = trie;
			}
		}
		else
			_data = data;
	}

	// Retrieves a list of words that a given (sub)trie holds
	template <typename T>
	void Trie<T>::getWords(Wordset& words) const {
		std::string buffer;
		words.clear();
		getWords(words, buffer);
	}

	// Retrievs a list of words which start from a given prefix
	template <typename T>
	void Trie<T>::getWordsWithPrefix(const std::string& prefix, Wordset& words) const {
		std::string buffer;
		words.clear();
		getWordsWithPrefix(prefix, words, buffer);
	}

	/*
	Returns a terminating node for a given word,
	nullptr if there's no such word in a trie
	*/
	template <typename T>
	const Trie<T>* Trie<T>::getNode(const std::string& word) const {
		// if no input, return current node
		if (word.empty())
			return this;

		const Trie* leaf = this;
		std::string buffer = word;
		while (buffer.length() > 0) {
			const auto& it = leaf->_leaves.find(buffer[0]);
			if (it != leaf->_leaves.end()) {
				leaf = it->second;
				buffer = buffer.substr(1);
			} else
				return nullptr;
		}

		return leaf;
	}

	template <typename T>
	void Trie<T>::getWords(Wordset& words, std::string& buffer) const {
		if (_mark)
			words.insert(buffer);

		for (const auto& it : _leaves) {
			string s = buffer + it.first;
			if (it.second->_mark)
				words.insert(s);

			it.second->getWords(words, s);
		}
	}

	template <typename T>
	void Trie<T>::getWordsWithPrefix(const std::string& prefix, Wordset& words, std::string& buffer) const {
		if (prefix.empty()) {
			getWords(words, buffer);
			return;
		}

		const std::string& remainder = prefix.substr(1);
		const char ch = prefix[0];
		if ((const auto& it = _leaves.find(ch)) != _leaves.end()) {
			const Trie const * leaf = it->second;
			buffer += prefix[0];
			leaf->getWordsWithPrefix(remainder, words, buffer);
		}
	}

	template <typename T>
	bool Trie<T>::isWord(const std::string& word) const {
		if (word.empty())
			return false;

		const Trie* node = getNode(word);
		if (!node)
			return false;

		return node->isMark();
	}
}; // namespace indigo

#endif // __trie_h__
