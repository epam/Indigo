#include "bingo_pg_build_engine.h"
#include "bingo_core_c.h"

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