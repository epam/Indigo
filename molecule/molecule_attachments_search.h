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

#include "base_cpp/obj_array.h"
#include "base_cpp/array.h"

#include "base_cpp/red_black.h"
#include "base_cpp/exception.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

template<typename K>
static K any(const RedBlackSet<K> &set) {
    return set.key(set.begin());
}

template<typename K>
static void copy(const RedBlackSet<K> &source, RedBlackSet<K> &target) {
    for (auto x = source.begin(); x != source.end(); x = source.next(x)) {
        target.insert(source.key(x));
    }
}

template<typename K, typename V>
static void copy(const RedBlackMap<K,V> &source, RedBlackMap<K,V> &target) {
    for (auto i = source.begin(); i != source.end(); i = source.next(i)) {
        target.insert(source.key(i), source.value(i));
    }
}

typedef RedBlackSet<int> Set;
typedef RedBlackMap<int, Set*> RawMultiMap;
typedef RedBlackMap<int, int>  Map;

class MultiMap {
public:
    explicit MultiMap() {}
    MultiMap(const MultiMap &other) : MultiMap(other.map) {}
    MultiMap(const RawMultiMap &other) { copy(other, map); }
    ~MultiMap() {}

    int  size() const;

    void insert(int key, int value);
    void insert(int key, const Array<int> &values);

    void remove(int key, int value);
    void remove(int key);

    const Set& operator[](int key) const;

    const char* print(const char *delim) const;
    const char* print() const;

protected:
    static const Set& nil;
    RawMultiMap map;
};

typedef int   Node;
typedef Array<Node> Path;

class DLLEXPORT Topology {
public:
    explicit Topology(int size)
    : lim(0), CP_INIT {
        expand(size);
    }
    Topology(const Topology &other)
    : forward(other.forward), backward(other.backward),
      lim(other.lim), CP_INIT {
        copy(other.current, current);
        copy(other.used,    used);
        path.copy(other.path);
    }
    ~Topology() {}

    const Path& history() const;
    const Set & pending() const;
    const Set & satisfied() const;

    void depends(Node source, Node target);
    bool satisfy(Node source);

    bool finished() const;

    const char* print() const;

    DECL_ERROR;

protected:
    void expand(int nlim);

    MultiMap forward;
    MultiMap backward;
    Set      current;
    Set      used;
    Path     path;
    int      lim;

    CP_DECL;
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

static IntervalFilter join(const IntervalFilter &a, const IntervalFilter &b) {
    return a.join(b);
}

typedef Array<IntervalFilter> IntervalFilters;

class DLLEXPORT OccurrenceRestrictions {
public:
    explicit OccurrenceRestrictions(int groupsN)
    : groups(groupsN), CP_INIT {
        restrictions.resize(groups);
        restrictions.fill(DEFAULT);
    }
    OccurrenceRestrictions(const OccurrenceRestrictions &other)
    : groups(other.groups), CP_INIT {
        restrictions.resize(groups);
        for (int i = 0; i < other.restrictions.size(); i++) {
            restrictions[i] = other.restrictions[i];
        }
    }
    ~OccurrenceRestrictions() {}

    void free(int group);
    void set(int group, const IntervalFilter &f);
    void set(int group, const Array<int>     &f);

    const IntervalFilter& operator[](int group) const {
        return restrictions[group - 1];
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

static const char* print(const Set &set) {
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
static const char* print(const Set *set) {
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

static const char* print(const MultiMap &map, const char *delim) {
    return print(map, delim);
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
    explicit Iterator()    { printf("Iterator<T>::<constructor>\n"); }
    virtual ~Iterator()    { printf("Iterator<T>::<destructor>\n");  }
    virtual bool hasNext() { throw Error("hasNext() is pure abstract"); }
    virtual T next()       { throw Error("next() is pure abstract"); }
    //pseudo-abstract class for simplicity of interfaces
};

template<typename T>
class DLLEXPORT Iterable {
public:
    virtual ~Iterable() = 0 {}
    virtual Iterator<T>* iterator() = 0;
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
class DLLEXPORT MapIterable : public Iterable<Y> {
public:
    explicit MapIterable(std::function<Y(X)> f, Iterable<X>* xs, bool clean = false)
        : f(f), xs(xs), clean(clean) {}
    virtual ~MapIterable() { delete xs; this->release(); }

    virtual Iterator<Y>* iterator() {
        return remember(map(f, xs->iterator(), clean));
    }
protected:
    std::function<Y(X)> f;
    Iterable<X>* const xs;

    CLEANER(Iterator<Y>*)
};

template<typename X, typename Y>
static Iterator<Y>* map(std::function<Y(X)> f, Iterator<X>* xs, bool clean = false) {
    return new MapIterator<X, Y>(f, xs, clean);
}

template<typename X, typename Y>
static Iterable<Y>* map(std::function<Y(X)> f, Iterable<X>* xs, bool clean = false) {
    return new MapIterable<X, Y>(f, xs, clean);
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
        if (xss->hasNext()) { xs = xss->next();  }
        else                { xs = nullptr; }
    }

    Iterator<Iterator<X>*>* const xss;
    Iterator<X>* xs = nullptr;
};

template<typename X>
class DLLEXPORT JoinIterable : public Iterable<X> {
public:
    explicit JoinIterable(Iterable<Iterable<X>*>* xss) : xss(xss) {}
    virtual ~JoinIterable() { delete xs; }

    virtual Iterator<X>* iterator() {
        return new JoinIterator<X>(map(
            Iterable<Iterable<X>*>::iterator,
            xss)->iterator());
    }
protected:
    Iterable<Iterable<X>*>* const xss;
};

template<typename X>
static Iterator<X>* join(Iterator<Iterator<X>*>* xss) {
    return new JoinIterator<X>(xss);
}

template<typename X>
static Iterable<X>* join(Iterable<Iterable<X>*>* xss) {
    return new JoinIterable<X>(xss);
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

/* END Iterable */

struct Fragment {
    Fragment(int g, int f)
    : group(g), fragment(f) {}
    int group    = 0;
    int fragment = 0;
};

typedef RedBlackMap<int,Fragment> Attachment;

static const char* print(const Fragment &fr) {
    char *ptr = new char[16];
    sprintf(ptr, "{%d:%d}", fr.group, fr.fragment);
    return ptr;
}
static const char* print(const Attachment &at) {
    return indigo::print(at, "~>");
}

class DLLEXPORT Attachments : public Iterable<Attachment*> {
public:
    Attachments(int sitesN, int groupsN,
      const MultiMap& site2group, const MultiMap& group2site,
      const Map     & _group2size, const OccurrenceRestrictions& occurrences,
      const Topology& top) : n(sitesN), k(groupsN), top(top),
        site2group(site2group), group2site(group2site), 
        occurrences(occurrences), CP_INIT {
      copy(_group2size, group2size);
    }
    virtual ~Attachments() {
        release();
    }
    virtual Iterator<Attachment*>* iterator() {
        return remember(new AttachmentsIterator(this));
    }

    DECL_ERROR;

protected:
    const int n, k;

    const Topology top;
    const MultiMap site2group;
    const MultiMap group2site;
          Map      group2size;
    const OccurrenceRestrictions occurrences;

    class AttachmentsIterator : public Iterator<Attachment*> {
    public:
        explicit AttachmentsIterator(Attachments *obj) : self(obj) {}
        virtual ~AttachmentsIterator() {}
        virtual bool hasNext()     { return true; }
        virtual Attachment* next() {
            return nullptr;//todo
        }

        Attachment* search() {
            Topology top(self->top);

            printf("%d sites, %d groups\n", self->n, self->k);
            printf("===================\n");
            printf("Site->Group: %s\nGroup->Site: %s\n", self->site2group.print(), self->group2site.print());
            printf("Group->Size: %s\nOccurrences: %s\nTopology: %s\n", indigo::print(self->group2size), self->occurrences.print(),
                top.print());
            printf("===================\n");

            Attachment* result = new Attachment();
            Attachment& at     = *result; //site  -> group
            Array<int> ocs;               //group -> occurrences
            ocs.resize(1 + self->k);
            ocs.fill(0);

            auto select = [&top]() {
                const Set* result = &top.pending();
                if (result->size() < 1) {
                    result = &top.satisfied();
                    printf("satisfied groups are chosen\n");
                } else {
                    printf("pending groups are chosen\n");
                }
                return result;
            };

            const Set* ptr;
            while (at.size() < self->n && (ptr = select())->size() > 0) {
                const Set &groups = *ptr;
                assert(groups.size() > 0);

                int group = any(groups); //todo: choose wisely           (!)
                const Set &sites = self->group2site[group];
                if (sites.size() < 1) { /* todo: continue */ }

                printf("group %d\n", group);

                int oc = ocs[group];
                int dist0 = self->occurrences[group][oc];
                int dist1 = self->occurrences[group][oc + 1];
                //todo: occurrence restrictions

                const int  site = any(sites); //todo: choose wisely      (!)
                printf("site %d\n", site);

                const int  size = self->group2size.at(group);
                const int  frag = 0; //todo: choose wisely               (!)
                printf("frag %d\n", frag);

                at.insert(site, { group, frag });
                top.satisfy(group);
            }
            assert(top.finished());
            return result;
        }

    protected:
        Attachments *self;
    };

    CP_DECL;

    CLEANER(Iterator<Attachment*>)
};

}

#ifdef _WIN32
#endif

#endif