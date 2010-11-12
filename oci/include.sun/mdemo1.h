#ifndef MDEMO1_ORACLE
# define MDEMO1_ORACLE

#ifndef OTT_USERCODE_START
# define OTT_USERCODE_START
#endif

#ifndef OTT_USERCODE_END
# define OTT_USERCODE_END
#endif

#ifndef OCCI_ORACLE
# include <occi.h>
#endif

OTT_USERCODE_START
#include "mymdemo1.h"
OTT_USERCODE_END


/************************************************************/
//  generated declarations for the FULL_NAME_O object type.
/************************************************************/

class FULL_NAME_O : public oracle::occi::PObject {

protected:

   OCCI_STD_NAMESPACE::string first_name;
   OCCI_STD_NAMESPACE::string last_name;

public:

   void *operator new(size_t size);

   void *operator new(size_t size, const oracle::occi::Connection * sess,
      const OCCI_STD_NAMESPACE::string& table);

   void *operator new(size_t, void *ctxOCCI_);

   OCCI_STD_NAMESPACE::string getSQLTypeName() const;

   FULL_NAME_O();

   FULL_NAME_O(void *ctxOCCI_) : oracle::occi::PObject (ctxOCCI_) { };

   static void *readSQL(void *ctxOCCI_);

   virtual void readSQL(oracle::occi::AnyData& streamOCCI_);

   static void writeSQL(void *objOCCI_, void *ctxOCCI_);

   virtual void writeSQL(oracle::occi::AnyData& streamOCCI_);

};

OTT_USERCODE_START

class FullName : public FULL_NAME_O {
public:
  FullName(string FirstName, string LastName);

  void displayInfo();
  const string getFirstName() const { return first_name;}
} ;

OTT_USERCODE_END

/************************************************************/
//  generated declarations for the ADDRESS_O object type.
/************************************************************/

class ADDRESS_O : public oracle::occi::PObject {

protected:

   OCCI_STD_NAMESPACE::string state;
   OCCI_STD_NAMESPACE::string zip;

public:

   void *operator new(size_t size);

   void *operator new(size_t size, const oracle::occi::Connection * sess,
      const OCCI_STD_NAMESPACE::string& table);

   void *operator new(size_t, void *ctxOCCI_);

   OCCI_STD_NAMESPACE::string getSQLTypeName() const;

   ADDRESS_O();

   ADDRESS_O(void *ctxOCCI_) : oracle::occi::PObject (ctxOCCI_) { };

   static void *readSQL(void *ctxOCCI_);

   virtual void readSQL(oracle::occi::AnyData& streamOCCI_);

   static void writeSQL(void *objOCCI_, void *ctxOCCI_);

   virtual void writeSQL(oracle::occi::AnyData& streamOCCI_);

OTT_USERCODE_START

    ADDRESS_O(string state_i, string zip_i);
    void displayInfo();

OTT_USERCODE_END

};

/************************************************************/
//  generated declarations for the PERSON_O object type.
/************************************************************/

class PERSON_O : public oracle::occi::PObject {

protected:

   oracle::occi::Number id;
   FullName * name;
   oracle::occi::Ref< ADDRESS_O > addr;

public:

   void *operator new(size_t size);

   void *operator new(size_t size, const oracle::occi::Connection * sess,
      const OCCI_STD_NAMESPACE::string& table);

   void *operator new(size_t, void *ctxOCCI_);

   OCCI_STD_NAMESPACE::string getSQLTypeName() const;

   PERSON_O();

   PERSON_O(void *ctxOCCI_) : oracle::occi::PObject (ctxOCCI_) { };

   static void *readSQL(void *ctxOCCI_);

   virtual void readSQL(oracle::occi::AnyData& streamOCCI_);

   static void writeSQL(void *objOCCI_, void *ctxOCCI_);

   virtual void writeSQL(oracle::occi::AnyData& streamOCCI_);

};

OTT_USERCODE_START

class Person : public PERSON_O {
public:
  Person(int id_i,
         FullName* name_i,
         Ref<ADDRESS_O>& addr_i);
  void move(const Ref<ADDRESS_O>& new_addr);
  void displayInfo();
};

OTT_USERCODE_END

#endif
