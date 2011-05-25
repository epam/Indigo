#include "bingo_pg_build_engine.h"
#include "bingo_core_c.h"

#include "base_cpp/tlscont.h"
#include "base_cpp/array.h"

#include "bingo_pg_index.h"

using namespace indigo;


BingoPgBuildEngine::BingoPgBuildEngine():
_bufferIndexPtr(0) {
   _bingoSession = bingoAllocateSessionID();
}

BingoPgBuildEngine::~BingoPgBuildEngine(){
   bingoReleaseSessionID(_bingoSession);
}

void BingoPgBuildEngine::_setBingoContext() {
   bingoSetSessionID(_bingoSession);
   bingoSetContext(0);
}

void BingoPgBuildEngine::loadDictionary(BingoPgIndex& bingo_index) {
   _setBingoContext();

   QS_DEF(Array<char>, dict);
   bingo_index.readDictionary(dict);
   bingoSetConfigBin("cmf_dict", dict.ptr(), dict.sizeInBytes());
}

const char* BingoPgBuildEngine::getDictionary(int& size) {
   _setBingoContext();

   const char* dict_buf;

   bingoGetConfigBin("cmf-dict", &dict_buf, &size);

   return dict_buf;
}