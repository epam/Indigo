#ifndef _BINGO_PG_BUILD_ENGINE_H__
#define	_BINGO_PG_BUILD_ENGINE_H__

/*
 * Interface class for procession fingerprint data
 */

#include "bingo_postgres.h"
#include "base_cpp/auto_ptr.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
class BingoPgFpData;


class BingoPgBuildEngine {
public:
   BingoPgBuildEngine();
   virtual ~BingoPgBuildEngine();

   virtual bool processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>&){return true;}

   virtual int getType() const {return 0;}
   virtual int getFpSize() {return 0;}

   virtual void prepareShadowInfo(){}
   virtual void insertShadowInfo(BingoPgFpData&){}
   virtual void finishShadowProcessing(){}

   virtual void loadDictionary(BingoPgIndex&){}
   virtual const char* getDictionary(int& size){size = 0; return 0;}

private:
   BingoPgBuildEngine(const BingoPgBuildEngine&); //no implicit copy
protected:
   void _setBingoContext();
   qword _bingoSession;
   BingoPgIndex* _bufferIndexPtr;
};



#endif	/* BINGO_PG_BUILD_ENGINE_H */

