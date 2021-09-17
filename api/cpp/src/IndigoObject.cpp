#include "IndigoObject.h"

#include <indigo.h>

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <thread>
#include <sstream>
#endif

using namespace indigo_cpp;

IndigoObject::IndigoObject(const int id, const IndigoSession& indigo) : id(id), indigo(indigo)
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": IndigoObject(" << indigo.getSessionId() << ", " << id << ")\n";
    std:: cout << ss.str();
#endif
    indigo._checkResult(id);
}

IndigoObject::~IndigoObject()
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": ~IndigoObject(" << indigo.getSessionId() << ", " << id << ")\n";
    std:: cout << ss.str();
#endif
    indigo.setSessionId();
    indigoFree(id);
}
