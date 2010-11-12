/* Copyright (c) 2000, 2002, Oracle Corporation.  All rights reserved.  */

/* 
   NAME 
     occiObjects.h - header file for OCCI object classes

   DESCRIPTION 
     Class definitions for Ref, RefAny, AnyData

   RELATED DOCUMENTS 
     <note any documents related to this facility>
 
   EXPORT FUNCTION(S) 
     <external functions declared for use outside package - one-line 
     descriptions>

   INTERNAL FUNCTION(S)
     <other external functions declared - one-line descriptions>

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   aahluwal    06/04/02 - bug 2360115
   rvallam     01/18/02 - 
   vvinay      12/24/01 - declarations for setNull() and isClear() in RefImpl
   rvallam     11/17/01 - implemented transactional consistency of refs
   gayyappa    10/01/01 - reserve memory for vectors in set/getVector 
   rkasamse    07/31/01 - add PObject::new(size_t, void*)
   gayyappa    07/17/01 - use C style comment in end of #ifdef WIN32COMMON.
   gayyappa    06/20/01 - fix linux porting bug 1801312.
   rvallam     06/14/01 - replace call to get(set)VectorOfRefs in 
                          get(set)Vector for Ref<T> with code in 
                          get(set)VectorOfRefs
   rvallam     11/06/01 - renamed internal methods in get/setVector to 
                          get(set)VectorOfPObjects/get(set)VectorOfOCIRefs
                          as part of NT porting fix
                          added destructor to AnyData
   rratnam     06/07/01 - fixed bug 1816387.
   rvallam     06/07/01 - fixed bug 1811749 :replaced .data() to .c_str()
                          for string to char* conversion.
   rvallam     04/30/01 - modified const methods in Ref (->, *, ptr)
   rvallam     04/12/01 - passed dummy argument OCCIPOBJECT in getVector
                          of AnyData for PObject *
   rvallam     04/09/01 - fixed bug 1721365- call RefAny default constructor
                          for a NULL Ref in operator RefAny
   gayyappa    03/29/01 - remove methods for get/set int/float/double/unsigned 
                          int , wasLastAttrNull, setAttrNull , geSQLTypeName
                          from AnyData 
                          as OTT does not use them.
   gayyappa    03/15/01 - add OCCItype parameter to getVector for OCCIRef.
   rratnam     03/15/01 - fixed set and getRef / get and setVector for NULL 
                          Refs, fixed NT compilation errors
   rratnam     03/13/01 - changed AnyData constructor to take a freeImg flag,
   rkasamse    03/15/01 - pass OCCI_SQLT_REF to getVector
   chliang     03/05/01 - disable olint.
   rvallam     03/01/01 - changed getSessionPtr() to getConnection()
                          added const methods for dereferencing in Ref
   gayyappa    02/23/01 - correct template code for setVector
   gayyappa    12/13/00 - remove allocator from list member for PObject.
                          bug#1529973
                          nullify object pointer in clear method of Ref
   rvallam     11/08/00 - make RefAny constructor public
   gayyappa    08/21/00 - replace objPtr by objptr.
                          move templated get/set vector code of anydata 
                          to header.
   rkasamse    08/07/00 - make getVector friend of RefAny
   rkasamse    07/11/00 - take void* instead of AnyDataCtx*
   rkasamse    07/26/00 - make ResultSetImp friend of RefAny
   rratnam     06/19/00 - added getConnection in PObject 
   rvallam     06/13/00 - added Ref<T> and occiRefImpl code
   rvallam     06/05/00 - to add the Ref<T> code
   kmohan      06/02/00 -
   kmohan      05/31/00 - Datamember Connection * changed to ConnectionImpl * 
                          in class RefAny
   kmohan      04/11/00 - Ref, RefAny, AnyData class definitions
                          added
   rkasamse    04/03/00 - header (interface) files for OCCI Objects clases
   rkasamse    04/03/00 - Creation

*/

#ifndef _olint   /* disable olint check */

#ifndef OCCIOBJECTS_ORACLE
# define OCCIOBJECTS_ORACLE

#ifndef OCCICOMMON_ORACLE
#include <occiCommon.h>
#endif

namespace oracle {
namespace occi {
struct AnyDataCtx {
   ConnectionImpl *occiSession;
   OCIAnyData *anyData;
   dvoid *objHeader;
   ub4 errNum;
};
typedef struct AnyDataCtx AnyDataCtx;

class PObject
{
  public:
        enum LockOption {OCCI_LOCK_WAIT, OCCI_LOCK_NOWAIT};
        enum UnpinOption {OCCI_PINCOUNT_DECR, OCCI_PINCOUNT_RESET};
        static void destroy(void *);
        PObject();
        PObject(const void *ctx);
        PObject(const PObject& obj);
        virtual ~PObject();
        PObject& operator=(const PObject& obj);
        void *operator new(size_t size);
        void *operator new(size_t size, const Connection *x,
                           const OCCI_STD_NAMESPACE::string& tablename, 
                           const char *typeName);
        void *operator new(size_t size, void *adctx);
        void operator delete(void *obj, size_t size);
        RefAny getRef() const;
        bool isLocked() const;
        void unpin(UnpinOption mode=OCCI_PINCOUNT_DECR);
        void pin();
        void lock(PObject::LockOption lock_option);
        void unmark();
        void flush();
        void markDelete();
        void markModified();
        bool isNull() const;
        void setNull();
        const Connection *getConnection() const;
        virtual OCCI_STD_NAMESPACE::string getSQLTypeName() const = 0;
        virtual void writeSQL(AnyData& stream) = 0;
        virtual void readSQL(AnyData& stream) = 0;
  private:
        ConnectionImpl  *occiSession_;
        dvoid *objHeader_;
        ub2 customNewed_;
        enum {CUSTOM_NEWED = 0x5cde};
        ub2 flags_;
        enum {NULL_INFO = 0x0001, GARBAGE_COLLECTED = 0x0002};
        // for future use 
        void *pobjectExt;
    
        friend class RefImpl;
};

class AnyData
{
    public:
    ~AnyData();
    AnyData(void *any) ;
    AnyData(const Connection *sessp, OCIAnyData *any, bool freeImg = true) ; 
 
    AnyData(const AnyData &src);
    AnyData& operator = (const AnyData &src);

    OCIAnyData* getOCIAnyData() const;
    const Connection* getConnection() const;


    bool isNull() const ;
    void setNull() ;
    OCCI_STD_NAMESPACE::string getString() const ;
    Blob getBlob() const ;
    Clob getClob() const ;
    Bfile getBfile() const ;
    Number getNumber() const ;
    Bytes getBytes() const ;
    Date getDate() const ;
    Timestamp getTimestamp() const ;
    PObject *getObject() const ;
    RefAny getRef() const ;

    void setString(const OCCI_STD_NAMESPACE::string &str)  ;
    void setBlob(const Blob &blob) ;
    void setClob(const Clob &clob) ;
    void setBfile(const Bfile &bfile) ;
    void setNumber(const Number &n) ;
    void setBytes(const Bytes &bytes) ;
    void setDate(const Date &date) ;
    void setTimestamp(const Timestamp &timestamp)  ;
    void setObject(const PObject *objptr) ;
    void setRef(const RefAny &ref) ;
    
    private:

                                 
    // private data members
    Ptr<AnyDataImpl> anyDataImplPtr;              
   

};

template <class T>
class Ref
{
  public:

  Ref();
  Ref(const T *obj) ;
  Ref(const RefAny &refAny) ;
  Ref(const Ref<T> &src) ;
  Ref(const Connection *sessp, OCIRef *tref, bool copy=TRUE) 
  ;
 ~Ref();
  Ref<T>& operator=(const Ref<T> &src) 
  ;
  Ref<T>& operator=(const T *obj) ;
  T * operator->() ;
  T * ptr() ;
  T & operator *() ;
  const T * operator->() const;
  const T * ptr()  const;
  const T & operator *() const ;
  void markDelete() ;
  void unmarkDelete() ;
  void setNull();
  bool isNull() const;
  void clear() ;
  bool isClear() const;
  void setPrefetch(const OCCI_STD_NAMESPACE::string &typeName, 
                   unsigned int depth);
  void setPrefetch(unsigned int depth) ;
  void setLock(LockOptions );
  operator RefAny() const;
  OCIRef *getRef() const;
  const Connection *getConnection() const;
  bool operator == (const Ref<T> &ref) const;
  bool operator != (const Ref<T> &ref) const;
  bool operator == (const RefAny &refAnyR) const ;
  bool operator != (const RefAny &refAnyR) const ;

  void destroy();

 private:

  RefImpl  *rimplPtr;
};


class RefImpl
{
  public:

  RefImpl();
  RefImpl(PObject *obj) ;
  RefImpl(const RefAny &refAny) ;
  RefImpl(const RefImpl &src) ;
  RefImpl(const Connection *sessp, OCIRef *tref, 
  bool copy=TRUE) ;
  ~RefImpl();
  bool isNull() const ;
  void setNull() ;
  void markDelete() ;
  void unmarkDelete() ;
  void clear() ;
  bool isClear() const ;
  void setPrefetch(const OCCI_STD_NAMESPACE::string &typeName, 
                   unsigned int depth) ;
  void setPrefetch(unsigned int depth) ;
  void setLock(LockOptions lckOption) ;
  PObject *pin() ;
  void unpin(PObject *obj) ;
  void setRefFromObjPtr(const PObject *obj) ;
  OCIRef* getRef() const;
  void setRefImpl(RefImpl *rptr);
  const Connection * getConnection() const;
  bool operator == (const RefImpl &refI) const ;
  bool operator == (const RefAny &refAnyR) const ;
  void assignObj(PObject *newObjPtr) ;
  // added following methods
  bool isEqual(PObject *obj);
  void operator = ( const RefImpl &src);
  void destroy();

 private:

  OCIRef *ref;
  const ConnectionImpl *sessp;
  OCIComplexObject *corhp;
  OCCI_STD_NAMESPACE::list<void *> descriptorList;
  LockOptions lockOption;
  // added data member for object header
  dvoid *objHeader;
};


class RefAny
{
  public:

  RefAny();
  RefAny (const Connection *sessptr, const OCIRef *ref);
  ~RefAny() ;
  RefAny(const RefAny& src) ;
  RefAny& operator=(const RefAny& src) ;
  void markDelete() ;
  void unmarkDelete() ;
  void clear() ;
  bool isNull() const;
  OCIRef * getRef() const;
  const Connection * getConnection() const;
  bool operator == (const RefAny &refAnyR) const;
  bool operator != (const RefAny &refAnyR) const;

  private:

  OCIRef *ref;
  const ConnectionImpl *sessp;
  // for future use
  void *refanyExt;

  friend RefAny MetaData::getRef(MetaData::AttrId) const;
  friend RefAny PObject::getRef() const;
  friend class AnyDataImpl;
  friend class ResultSetImpl;
  friend class StatementImpl;
  friend void getVector(const ResultSet  *rs,
                        unsigned int colIndex,
                        OCCI_STD_NAMESPACE::vector<RefAny> &vect) ;
  friend void getVector(const Statement  *stmt,
                        unsigned int colIndex,
                        OCCI_STD_NAMESPACE::vector<RefAny> &vect) ;
};

template <class T>
Ref<T>::Ref()
{
   rimplPtr = new RefImpl();
}

template <class T>
Ref<T>::Ref(const T *obj)
{
   rimplPtr = new RefImpl((PObject *)obj);
}

template <class T>
Ref<T>::Ref(const RefAny &refAny)

{
  rimplPtr = new RefImpl(refAny);
}

template <class T>
Ref<T>::Ref(const Ref<T>& src)

{
    rimplPtr = new RefImpl(*(src.rimplPtr));
}

template <class T>
Ref<T>::Ref(const Connection *sessp, OCIRef *tref, bool copy)

{
   rimplPtr = new RefImpl(sessp, tref, copy);
}

template <class T>
Ref<T>::~Ref()
{
  delete rimplPtr;
}


template <class T>
Ref<T>& Ref<T>::operator=(const Ref<T> &src)
{
   if (&src == this)
     return *this;
   *rimplPtr = *(src.rimplPtr);
   return *this;
}

template <class T>
Ref<T>& Ref<T>::operator=(const T *obj)
{
  if (rimplPtr->isEqual((PObject *)obj))
    return *this;
  rimplPtr->assignObj((PObject *)obj);
  return *this;
}

template <class T>
T* Ref<T>::operator->()
{
  return ((T *)rimplPtr->pin());
}

template <class T>
T* Ref<T>::ptr()
{
  return ((T *)rimplPtr->pin());
}

template <class T>
T& Ref<T>::operator * ()
{
  return ((T &)(*(rimplPtr->pin())));
}

template <class T>
const T* Ref<T>::operator->() const
{
  return ((const T *)rimplPtr->pin());
}

template <class T>
const T* Ref<T>::ptr() const
{
  return ((const T *)rimplPtr->pin());
}

template <class T>
const T& Ref<T>::operator * () const
{
  return ((const T &)(*(rimplPtr->pin())));
}

template <class T>
void Ref<T>::markDelete ()
{
   rimplPtr->markDelete();
}

template <class T>
void Ref<T>::unmarkDelete ()
{
   rimplPtr->unmarkDelete();
}

template <class T>
void Ref<T>::setNull()
{
   rimplPtr->setNull();
}

template <class T>
bool Ref<T>::isNull() const
{
  return rimplPtr->isNull();
}

template <class T>
void Ref<T>::clear ()
{
   rimplPtr->clear();
}

template <class T>
bool Ref<T>::isClear() const
{
   return rimplPtr->isClear();
}

template <class T>
void Ref<T>::setPrefetch (const OCCI_STD_NAMESPACE::string &typeName,
unsigned int depth)

{
  rimplPtr->setPrefetch(typeName,depth);
}

template <class T>
void Ref<T>::setPrefetch (unsigned int depth)

{
  rimplPtr->setPrefetch(depth);
}

template <class T>
void Ref<T>::setLock (LockOptions lckOption)
{
  rimplPtr->setLock(lckOption);
}

template <class T>
OCIRef* Ref<T>::getRef() const
{
  return (rimplPtr->getRef());
}

template<class T>
const Connection* Ref<T>::getConnection () const
{
  return (rimplPtr->getConnection());
}

template <class T>
Ref<T>::operator RefAny () const
{
  if (isNull())
    return  RefAny();
  return (RefAny(rimplPtr->getConnection(), rimplPtr->getRef()));
}

template <class T>
bool Ref<T>::operator ==(const Ref<T> &ref) const

{
  return ( (*rimplPtr) == (*(ref.rimplPtr)) );
}

template <class T>
bool Ref<T>::operator !=(const Ref<T> &ref) const

{
  return ( !((*rimplPtr) == (*(ref.rimplPtr))) );
}

template <class T>
bool Ref<T>::operator == (const RefAny & refAnyR) const

{
  return ( (*rimplPtr) == refAnyR );
}

template <class T>
bool Ref<T>::operator != (const RefAny & refAnyR) const

{
  return ( !((*rimplPtr) == refAnyR ));
}

template <class T>
void Ref<T>::destroy()
{
   rimplPtr->destroy();
}
/*---------------------------------------------------------------------------
                PROTOTYPES USED BY FUNCTION TEMPLATES
 ---------------------------------------------------------------------------*/
  void getVectorOfOCIRefs( const AnyData &any, 
  OCCI_STD_NAMESPACE::vector<void *> &vect);
  void getVectorOfPObjects( const AnyData &any,
  OCCI_STD_NAMESPACE::vector< PObject* > &vect) ;
  void setVectorOfOCIRefs( AnyData &any, 
  const OCCI_STD_NAMESPACE::vector<void *> &vect,
  const OCCI_STD_NAMESPACE::vector< OCIInd> &vec_ind) ;
  void setVectorOfPObjects( AnyData &any, 
  const OCCI_STD_NAMESPACE::vector< PObject* > &vect) ;

/*---------------------------------------------------------------------------
                           EXPORT FUNCTIONS
  ---------------------------------------------------------------------------*/

/*------------------- getVector for POBject----------------------------*/
/*
   NAME
      getVector - overloaded function. Retrieves the attribute in the
   current position as a vector of PObject

   PARAMETERS
      any - AnyData
      vect- reference to vector of PObject (OUT parameter).

   DESCRIPTION
   Retrieves the attribute in the current position as a vector
   of PObject
   The attribute at the current position should be a collection
   type (varray or nested table). The SQL type of the elements in
   the collection should be compatible with PObject

   RETURNS
     nothing

   NOTES
     compatible SQL types : user defined types (SQLT_NTY) etc.
*/

#ifdef WIN32COMMON
// and other platforms that do not support
// partial function template specialization
 template <class T>
 void getVector(const AnyData &any, OCCI_STD_NAMESPACE::vector<T> &vect)
 {
   OCCI_STD_NAMESPACE::vector< PObject *> vec_pobj;
   getVectorOfPObjects( any, vec_pobj);

   vect.clear();
   unsigned int size= vec_pobj.size();
   vect.reserve( size );
   for( unsigned int i=0; i< size; i++)
      vect.push_back( (T)vec_pobj[i] );
 }
#else
 template <class T>
 void getVector(const AnyData &any, OCCI_STD_NAMESPACE::vector<T*> &vect) 
 {
   OCCI_STD_NAMESPACE::vector< PObject *> vec_pobj;
   getVectorOfPObjects( any, vec_pobj);

   vect.clear();
   unsigned int size= vec_pobj.size();
   vect.reserve( size );
   for( unsigned int i=0; i< size; i++)
      vect.push_back( (T*)vec_pobj[i] );
 }
#endif /* end of #ifdef WIN32COMMON */

 /*------------------- getVector for Ref<T>----------------------------*/
/*
   NAME
      getVector - overloaded function. Retrieves the attribute in the
   current position as a vector of PObject

   PARAMETERS
      any - AnyData
      vect- reference to vector of PObject (OUT parameter).

   DESCRIPTION
   Retrieves the attribute in the current position as a vector
   of PObject
   The attribute at the current position should be a collection
   type (varray or nested table). The SQL type of the elements in
   the collection should be compatible with PObject

   RETURNS
     nothing

   NOTES
     compatible SQL types : user defined types (SQLT_NTY) etc.
*/
#ifndef WIN32COMMON
 template <class T>
 void getVector(const AnyData &any,OCCI_STD_NAMESPACE::vector< Ref<T> > &vect) 
 {
   OCCI_STD_NAMESPACE::vector< void *> vec_ref;
   getVectorOfOCIRefs( any, vec_ref);

   vect.clear();
   unsigned int size = vec_ref.size();
   vect.reserve( size );
   const Connection *sess = any.getConnection();

   for (unsigned int i=0; i< size; i++)
   {
     if (vec_ref[i] == (OCIRef *)0)
       vect.push_back(Ref<T>());     // pushing a default-constructed Ref
     else
       vect.push_back(Ref<T>(sess, (OCIRef *)vec_ref[i], FALSE));
   }
 }
#endif /* end of #ifndef WIN32COMMON  */
 
/*-----------------------setVector for PObject--------------------------*/
/*
   NAME
     setVector - overloaded function. sets the attribute in the current
   position of anydata with the vector elements.

   PARAMETERS
      none.

   DESCRIPTION
      sets the attribute in the current position in anydata with the
   vector elements.
  The attribute in the current position of anydata should be a
  collection type. If the collection type is a varray, the input vector
  size should be equal to the size of the varray. Also the SQL type of
  the collection's elements should be compatible with PObject.

   RETURNS
     nothing.

   NOTES
     compatible SQL types : SQLT_NTY  (user defined types).
*/
#ifdef WIN32COMMON
// and other platforms that do not support
// partial function template specialization

 template <class T>
 void setVector(AnyData &any, const OCCI_STD_NAMESPACE::vector<T> &vect) 
 {
   OCCI_STD_NAMESPACE::vector< PObject *> vec_pobj;
   unsigned int size= vect.size();
   vec_pobj.reserve( size );
   for( unsigned int i=0; i< size; i++)
      vec_pobj.push_back( vect[i] );
   setVectorOfPObjects( any, vec_pobj);

 }
 
#else

 template <class T>
 void setVector(AnyData &any, const OCCI_STD_NAMESPACE::vector<T*> &vect)
 {
   OCCI_STD_NAMESPACE::vector< PObject *> vec_pobj;
   unsigned int size= vect.size();
   vec_pobj.reserve( size );
   for( unsigned int i=0; i< size; i++)
      vec_pobj.push_back( vect[i] );
   setVectorOfPObjects( any, vec_pobj);

 }
#endif /* end of #ifdef WIN32COMMON */

/*-----------------------setVector for Ref<T>--------------------------*/
/*
   NAME
     setVector - overloaded function. sets the attribute in the current
   position of anydata with the vector elements.

   PARAMETERS
      none.

   DESCRIPTION
      sets the attribute in the current position in anydata with the
   vector elements.
  The attribute in the current position of anydata should be a
  collection type. If the collection type is a varray, the input vector
  size should be equal to the size of the varray. Also the SQL type of
  the collection's elements should be compatible with PObject.

   RETURNS
     nothing.

   NOTES
     compatible SQL types : SQLT_NTY  (user defined types).
*/
#ifndef WIN32COMMON
 template <class T>
 void setVector(AnyData &any, const OCCI_STD_NAMESPACE::vector< Ref<T> > &vect)
 {
   OCCI_STD_NAMESPACE::vector< void *> vec_ref;
   OCCI_STD_NAMESPACE::vector<OCIInd> vec_ind;

   unsigned int size= vect.size();
   vec_ref.reserve( size );
   vec_ind.reserve( size );
   for( unsigned int i=0; i< size; i++)
   {
      vec_ref.push_back( vect[i].getRef() );
      vec_ind.push_back(vect[i].isNull() ? OCI_IND_NULL : OCI_IND_NOTNULL);
   }
   setVectorOfOCIRefs( any, vec_ref, vec_ind);

 }
#endif  /* end of #ifndef WIN32COMMON */
 
// Platform independent get/setVectorOfRefs method added 
// get(set)Vector of Ref<T> and get(set)VectorOfRefs are identical
// in functionality.

 /*------------------- getVectorOfRefs for Ref<T>----------------------------*/
/*
   NAME
      getVectorOfRefs - overloaded function. Retrieves the attribute in the
   current position as a vector of PObject

   PARAMETERS
      any - AnyData
      vect- reference to vector of PObject (OUT parameter).

   DESCRIPTION
   Retrieves the attribute in the current position as a vector
   of PObject
   The attribute at the current position should be a collection
   type (varray or nested table). The SQL type of the elements in
   the collection should be compatible with PObject

   RETURNS
     nothing

   NOTES
     compatible SQL types : user defined types (SQLT_NTY) etc.
*/

 template <class T>
 void getVectorOfRefs(const AnyData &any,
                      OCCI_STD_NAMESPACE::vector< Ref<T> > &vect)
 {
   OCCI_STD_NAMESPACE::vector< void *> vec_ref;
   getVectorOfOCIRefs( any, vec_ref);

   vect.clear();
   unsigned int size = vec_ref.size();
   vect.reserve( size );
   const Connection *sess = any.getConnection();

   for (unsigned int i=0; i< size; i++)
   {
     if (vec_ref[i] == (OCIRef *)0)
       vect.push_back(Ref<T>());     // pushing a default-constructed Ref
     else
       vect.push_back(Ref<T>(sess, (OCIRef *)vec_ref[i], FALSE));
   }
 }

/*-----------------------setVectorOfRefs for Ref<T>--------------------------*/
/*
   NAME
     setVectorOfRefs - overloaded function. sets the attribute in the current
   position of anydata with the vector elements.

   PARAMETERS
      none.

   DESCRIPTION
      sets the attribute in the current position in anydata with the
   vector elements.
  The attribute in the current position of anydata should be a
  collection type. If the collection type is a varray, the input vector
  size should be equal to the size of the varray. Also the SQL type of
  the collection's elements should be compatible with PObject.

   RETURNS
     nothing.

   NOTES
     compatible SQL types : SQLT_NTY  (user defined types).
*/

 template <class T>
 void setVectorOfRefs(AnyData &any, 
                      const OCCI_STD_NAMESPACE::vector< Ref<T> > &vect)

 {
   OCCI_STD_NAMESPACE::vector< void *> vec_ref;
   OCCI_STD_NAMESPACE::vector<OCIInd> vec_ind;

   unsigned int size= vect.size();
   vec_ref.reserve( size );
   vec_ind.reserve( size );
   for( unsigned int i=0; i< size; i++)
   {
      vec_ref.push_back( vect[i].getRef() );
      vec_ind.push_back(vect[i].isNull() ? OCI_IND_NULL : OCI_IND_NOTNULL);
   }
   setVectorOfOCIRefs( any, vec_ref, vec_ind);

 }


/*---------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
  ---------------------------------------------------------------------------*/


} /* end of namespace occi */
} /* end of namespace oracle */
#endif                                              /* OCCIOBJECTS_ORACLE */

#endif                                              /* _olint */
