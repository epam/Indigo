/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __molecule_attachments_search__
#define __molecule_attachments_search__

#include "assert.h"

#include "algorithm"
#include "functional"

#include "base_cpp/array.h"
#include "base_cpp/multimap.h"

#include "base_cpp/red_black.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"

namespace indigo {

template<typename K>
static void copy(const RedBlackSet<K> &source, RedBlackSet<K> &target) {
    target.clear();
    for (auto i = source.begin(); i != source.end(); i = source.next(i)) {
        target.insert(source.key(i));
    }
}

template<typename K, typename V>
static void copy(const RedBlackMap<K,V> &source, RedBlackMap<K,V> &target) {
    target.copy(source);
}

template<typename K, typename V>
static void copy(const RedBlackMap<K, V*> &source, RedBlackMap<K, V*> &target) {
    target.clear();
    for (auto i = source.begin(); i != source.end(); i = source.next(i)) {
        V* v = new V();
        copy(*source.value(i), *v);
        target.insert(source.key(i), v);
    }
}

class DLLEXPORT Topology {
public:
    explicit Topology(int size)
    : lim(0), CP_INIT {
        expand(size);
    }

    void copy(Topology &target) const {
        target.lim = lim;
        forward.copy(target.forward);
        backward.copy(target.backward);
        indigo::copy(current, target.current);
        indigo::copy(used, target.used);
        target.path.copy(path);
    }

    ~Topology() {}

    const Array<int>& history() const;
    const RedBlackSet<int>& pending() const;
    const RedBlackSet<int>& satisfied() const;

    void depends(int source, int target);
    bool satisfy(int source);
    void allow(int source);

    bool finished() const;

    void print(Array<char> &out, bool finalize = true) const;

    DECL_ERROR;

protected:
    void expand(int nlim);

    MultiMap<int,int> forward;
    MultiMap<int,int> backward;

    RedBlackSet<int>  current;
    RedBlackSet<int>  used;
    Array<int>        path;
    int               lim;

    CP_DECL;

private:
    Topology(const Topology &); // no implicit copy
};

#define swap1(l,r) \
    { auto t = l;  \
      l = r;       \
      r = t; }

#define swap5(ls,l0,l1,li,ln,rs,r0,r1,ri,rn,f) \
    if (!(f)) {           \
        printf("swap\n"); \
        swap1(ls,rs);     \
        swap1(l0,r0);     \
        swap1(l1,r1);     \
        swap1(li,ri);     \
        swap1(ln,rn);     \
    }

#define max4(w,x,y,z) std::max(std::max(w,x), std::max(y,z))
#define min4(w,x,y,z) std::min(std::min(w,x), std::min(y,z))

class DLLEXPORT IntervalFilter {
public:
    IntervalFilter(const IntervalFilter &other);
    IntervalFilter(int from, int to);
    IntervalFilter(int bits);
    ~IntervalFilter() { /* todo */ }

    int  distance(int x) const;
    bool match(int x)    const { return distance(x) == 0; }

    static const int MIN = 0;
    static const int MAX = INT_MAX;

    //possible to do this: IntervalFilter &f = range(3,5) && point(9) && gt(30)
    IntervalFilter operator&&(const IntervalFilter &other) const { return join(other); }
    IntervalFilter join(const       IntervalFilter &other) const;

    IntervalFilter& operator=(const IntervalFilter &other) {
        //todo: fix
        IntervalFilter *copy = new IntervalFilter(other);
        memcpy(this, copy, sizeof(*copy));
        return *this;
    }
    int operator[](int x) const { return distance(x); }

    const char* print() const;

    DECL_ERROR;

protected:
    explicit IntervalFilter(const Array<int> &array);

    Array<int> map; //todo: binary search is better

    CP_DECL;

private:
    static void append(Array<int> &target, const Array<int> &source, int from);
};

static IntervalFilter convert(const Array<int> &bits) {
    IntervalFilter f(bits[0]);
    for (auto i = 1; i < bits.size(); i++) {
        f = f && bits[i];
    }
    return f;
}

static IntervalFilter range(int l, int r) { return IntervalFilter(l, r); }
static IntervalFilter point(int n)        { return range(n, n); }
static IntervalFilter lt(int n)           { return range(INT_MIN, n - 1); }
static IntervalFilter gt(int n)           { return range(n + 1, INT_MAX); }

static IntervalFilter positive()          { return gt(0);  }
static IntervalFilter natural()           { return gt(-1); }

static IntervalFilter join(const IntervalFilter &a, const IntervalFilter &b) {
    return a.join(b);
}

typedef Array<IntervalFilter> IntervalFilters;

class DLLEXPORT OccurrenceRestrictions {
public:
    explicit OccurrenceRestrictions(int groupsN)
    : groups(groupsN), CP_INIT {
        restrictions.resize(groups+1);
        restrictions.fill(DEFAULT);
    }
    OccurrenceRestrictions(const OccurrenceRestrictions &other)
    : groups(other.groups), CP_INIT {
        restrictions.resize(groups+1);
        for (int i = 0; i < other.restrictions.size(); i++) {
            restrictions[i] = other.restrictions[i];
        }
    }
    ~OccurrenceRestrictions() {}

    void free(int group);
    void set(int group, const IntervalFilter &f);
    void set(int group, const Array<int>     &f);

    const IntervalFilter& operator[](int group) const {
        return restrictions[group];
    }

    const char* print() const;

    DECL_ERROR;

    static const IntervalFilter DEFAULT;

protected:
    IntervalFilters restrictions;
    int groups;

    CP_DECL;
};

template<typename T>
static const char* print(const T *t) {
    if (t == nullptr) { return "<nullptr>";  }
    return print(*t);
}

static const char* print(int x) {
    char* ptr = new char[1024];
    const char* out = ptr;
    sprintf(ptr, "%d", x);
    return out;
}

static const char* print(const IntervalFilter &f) {
    return f.print();
}

static const char* print(const RedBlackSet<int> &set) {
    if (set.size() < 1) return "{}";

    char* ptr = new char[1024];
    const char* out = ptr;
    for (int i = set.begin(); i != set.end(); i = set.next(i)) {
        ptr += sprintf(ptr, "%s%d", i == set.begin() ? "{" : ", ", set.key(i));
    }
    ptr += sprintf(ptr, "}");
    return out;
}

template<>
static const char* print(const RedBlackSet<int> *set) {
    if (set == nullptr) { return "{}"; }
    return print(*set);
}

template<typename T>
static const char* print(const RedBlackMap<int, T> &map, const char *delim) {
    if (map.size() < 1) return "[]";

    char* ptr = new char[1024];
    const char* out = ptr;

    for (int i = map.begin(); i != map.end(); i = map.next(i)) {
        ptr += sprintf(ptr, "%s%d%s%s", i == map.begin() ? "[" : ", ",
            map.key(i), delim, print(map.value(i)));
    }
    ptr += sprintf(ptr, "]");

    return out;
}

template<typename T>
static const char* print(const RedBlackMap<int, T> &map) {
    return print(map, "->");
}

template<typename T>
static const char* print(const Array<T> &array) {
    char* ptr = new char[1024];
    const char* out = ptr;

    ptr += sprintf(ptr, "[");
    bool empty = true;
    for (int i = 0; i < array.size(); i++) {
        ptr += sprintf(ptr, "%s%s", empty ? "" : ", ", print(array[i]));
        empty = false;
    }
    ptr += sprintf(ptr, "]");

    return out;
}

class ErrorThrower {
public: DECL_ERROR;
};

/* BEGIN Iterable
   TODO: place into proper place or use something different
*/

template<typename T>
class DLLEXPORT Iterator : public ErrorThrower {
public:
    explicit Iterator()    { }
    virtual ~Iterator()    { }
    virtual bool hasNext() { throw Error("hasNext() is pure abstract"); }
    virtual T next()       { throw Error("next() is pure abstract"); }
    //pseudo-abstract class for simplicity of interfaces
};

template<typename T>
class DLLEXPORT Cleaner : public Iterator<T> {
public:
    explicit Cleaner(bool clean) {}
protected:
    T    remember(T t) { return t; }
    void release() {}
private:
    bool clean;
};

#define CLEANER(T) \
protected:                                                  \
    T* remember(T* ptr) {                                   \
        if (clean) { allocated.push(ptr); }                 \
        return ptr;                                         \
    }                                                       \
    void release() {                                        \
        for (int i = 0; i < allocated.size(); i++) {        \
            delete allocated[i];                            \
        }                                                   \
    }                                                       \
private:                                                    \
    Array<T*> allocated;                                    \
    bool clean;                                             \

template<typename T>
class DLLEXPORT Cleaner<T*> {
public:
    explicit Cleaner(bool clean) : clean(clean) {}
    virtual ~Cleaner() {};
    CLEANER(T)
};

template<typename X, typename Y>
class DLLEXPORT MapIterator : public Iterator<Y>, public Cleaner<Y> {
public:
    explicit MapIterator(std::function<Y(X)> f, Iterator<X>* xs, bool clean = false)
        : Cleaner(clean), f(f), xs(xs) {}
    ~MapIterator() { delete xs; this->release(); }

    virtual bool hasNext() { return xs->hasNext(); }
    virtual Y    next() { return remember(f(xs->next())); }

protected:
    std::function<Y(X)> f;
    Iterator<X>* const xs;
};

template<typename X, typename Y>
static Iterator<Y>* map(std::function<Y(X)> f, Iterator<X>* xs, bool clean = false) {
    return new MapIterator<X, Y>(f, xs, clean);
}

template<typename X>
class DLLEXPORT JoinIterator : public Iterator<X> {
public:
    explicit JoinIterator(Iterator<Iterator<X>*>* xss) : xss(xss) {}
    ~JoinIterator() { delete xss; }

    virtual bool hasNext() { advance(); return xs != nullptr && xs->hasNext(); }
    virtual X    next()    { advance(); return xs->next(); }

protected:
    void advance() {
        if (xs != nullptr && xs->hasNext()) { return; }
        if (xss->hasNext()) {
            xs = xss->next();
            if (!xs->hasNext()) {
                advance();
            }
        } else {
            xs = nullptr;
        }
    }

    Iterator<Iterator<X>*>* const xss;
    Iterator<X>* xs = nullptr;
};

template<typename X>
static Iterator<X>* join(Iterator<Iterator<X>*>* xss) {
    return new JoinIterator<X>(xss);
}

template<typename T>
class DLLEXPORT Nil : public Iterator<T> {
public:
    explicit Nil() {}
    virtual ~Nil() {}

    virtual bool hasNext() { return false; }
    virtual T    next()    { throw Iterator<T>::Error("no more elements"); }
};

template<typename T>
class DLLEXPORT Cons : public Iterator<T> {
public:
    explicit Cons(T head, Iterator<T>* tail) : head(head), tail(tail) {}
    virtual ~Cons() { delete tail; }

    virtual bool hasNext() { return used ? tail->hasNext() : true; }
    virtual T    next() {
        if (!used) {
            used = true;
            return head;
        }
        return tail->next();
    }

protected:
    const    T         head;
    Iterator<T>* const tail;

    bool used = false;
};

template<typename T>
static Iterator<T>* nil() { return new Nil<T>(); }

template<typename T>
static Iterator<T>* cons(T head, Iterator<T>* tail) { return new Cons<T>(head, tail); }

template<typename T>
static Iterator<T>* single(T value) { return cons(value, nil<T>()); }

template<typename T>
class DLLEXPORT Lazy {
public:
    explicit Lazy(std::function<T()> f) : f(f) {}
    ~Lazy() { if (t) { delete t; } }

    T get() {
        if (!t) { t = new T(); return *t = f(); }
        return *t;
    }

protected:
    T* t = nullptr;
    std::function<T()> f;
};

template<typename T>
class DLLEXPORT LazyCons : public Iterator<T> {
public:
    explicit LazyCons(T head, std::function<Iterator<T>*()> f) : head(head), tail(Lazy<Iterator<T>*>(f)) {}
    virtual ~LazyCons() {}

    virtual bool hasNext() {
        return used ? (tail.get())->hasNext() : true;
    }
    virtual T    next() {
        if (!used) {
            used = true;
            return head;
        }
        return tail.get()->next();
    }

protected:
    const         T    head;
    Lazy<Iterator<T>*> tail;

    bool used = false;
};

template<typename F, typename T>
static Iterator<T>* cons(T head, F f) {
    return new LazyCons<T>(head, f);
}

template<typename T>
class SetIterator : public Iterator<T> {
public:
    SetIterator(const RedBlackSet<T> &set) : set(set), i(set.begin()) {}
    virtual ~SetIterator() {}

    virtual bool hasNext() { return i != set.end(); }
    virtual T    next()    { T t = set.key(i); i = set.next(i); return t; }

protected:
    const RedBlackSet<T> &set;
    int i;
};

template<typename T>
static Iterator<T>* iterator(const RedBlackSet<T> &set) {
    return new SetIterator<T>(set);
}

class RangeIterator : public Iterator<int> {
public:
    RangeIterator(int from, int to) : from(from), to(to), i(from) {}
    virtual ~RangeIterator() {}

    virtual bool hasNext() { return i <= to; }
    virtual int  next()    { return i++; }
protected:
    const int from;
    const int to;

    int i;
};

static Iterator<int>* count(int from, int to) { return new RangeIterator(from, to); }

template<typename T>
static Iterator<T>* every(const RedBlackSet<T> &set) {
    return new SetIterator<T>(set);
}

/* END Iterable */

struct Fragment {
    Fragment(int g, int f)
      : group(g), fragment(f) {}
    int group;
    int fragment;
};

static const int      H = 0;
static const Fragment Hydrogen = {H, 0};

typedef RedBlackMap<int,Fragment> Attachment;

static const char* print(const Fragment &fr) {
    char *ptr = new char[16];
    sprintf(ptr, "{%d:%d}", fr.group, fr.fragment);
    return ptr;
}
static const char* print(const Attachment &at) {
    return indigo::print(at, "~>");
}

class DLLEXPORT Attachments {
public:
    Attachments(int sitesN, int groupsN,
      const MultiMap<int,int>& site2group, const MultiMap<int,int>& group2site,
      const RedBlackMap<int,int>& group2size, const OccurrenceRestrictions& occurrences,
      const Topology& top) : n(sitesN), k(groupsN), _top(n), _occurrences(occurrences), CP_INIT
    {
       top.copy(_top);
       site2group.copy(_site2group);
       group2site.copy(_group2site);
       copy(group2size, _group2size);

       Array<char> out;
       out.appendString("top: ", false);
       top.print(out, false);
       out.appendString("\nsite2group: ", false);
       print(site2group, out, false);
       out.appendString("\ngroup2site: ", false);
       print(group2site, out, true);
       printf("%s\n", out.ptr());
    }

    ~Attachments() {
        release();
    }

    Iterator<Attachment*>* iterator() {
        std::function<Attachment*(State)> peel = [](State &state) {
            Attachment* result = new Attachment();
            copy(*state.at, *result);
            return result;
        };
        return map(peel, search(init()));
    }

    DECL_ERROR;

protected:
    const int n, k;

    Topology _top;

    MultiMap<int,int>    _site2group;
    MultiMap<int,int>    _group2site;
    RedBlackMap<int,int> _group2size;

    const OccurrenceRestrictions _occurrences;

    struct State {
        const Topology   *top;
        const Attachment *at;
        const Array<int> *ocs;

        const int debug_depth;
    };

    State init() {
        Array<int> *ocs = new Array<int>();
        ocs->resize(1 + k);
        ocs->fill(0);

        Topology *top = new Topology(n);
        _top.copy(*top);

        return { top, new Attachment(), ocs, 0 };
    }

    Iterator<State>* search(State &state) {
        if (state.at->size() == n) {
            return single(state);
        }

        const RedBlackSet<int>* ptr = &state.top->pending();
        if (ptr->size() < 1) {
            ptr = &state.top->satisfied();
        }
        assert(ptr->size() > 0);
        auto groups = every(*ptr);

        std::function<Iterator<State>*(int)> group2states = [=](int group) {
            printf("DEBUG {depth=%d; group=%d; at=%s}\n", state.debug_depth, group, print(state.at));
            int occurs   = (*state.ocs)[group];
            int currDist = _occurrences[group][occurs];
            int nextDist = _occurrences[group][occurs + 1];
            //todo: occurrence restrictions

            auto frags = count(1, _group2size.at(group));
            auto sites = every(_group2site[group]);

            std::function<Iterator<State>*(int)> site2states = [=](int site) {
                std::function<Iterator<State>*(int)> attachAndRec = [=](int frag) {
                    Topology *top = new Topology(n);
                    state.top->copy(*top);
                    Attachment *at = new Attachment();
                    copy(*state.at, *at);
                    Array<int> *ocs = new Array<int>();
                    (*ocs).copy(*state.ocs);
                    (*ocs)[group] = 1 + (*ocs)[group];

                    at->insert(site, { group, frag });
                    top->satisfy(group);

                    State result = { top, at, ocs, state.debug_depth+1 };
                    return search(result);
                };
                return join(map(attachAndRec, frags));
            };
            return join(map(site2states, sites));
        };
        return join(map(group2states, groups));
    }

    CP_DECL;

    CLEANER(Iterator<Attachment*>)
};

}

#ifdef _WIN32
#endif

#endif