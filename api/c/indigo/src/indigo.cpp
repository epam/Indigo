/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "indigo_internal.h"
#include "indigo_version.h"

#include <atomic>
#include <clocale>

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molfile_saver.h"
#include "reaction/pathway_reaction_json_saver.h"
#include "reaction/reaction_json_saver.h"
#include "reaction/rxnfile_saver.h"

#include "indigo_abbreviations.h"

// #define INDIGO_DEBUG
// #define INDIGO_OBJECT_DEBUG

#ifdef INDIGO_DEBUG
#include <iostream>
#endif

DLLEXPORT _SessionLocalContainer<Indigo>& indigoSelf()
{
    static _SessionLocalContainer<Indigo> indigo_self;
    return indigo_self;
}

DLLEXPORT Indigo& indigoGetInstance()
{
    return indigoSelf().getLocalCopy();
}

CEXPORT const char* indigoVersion()
{
    return INDIGO_VERSION "-" INDIGO_PLATFORM;
}

CEXPORT const char* indigoVersionInfo()
{
    INDIGO_BEGIN
    {
        std::string version = indigoVersion();

        const std::string digits = "0123456789";
        const auto parse = [&](const auto& shift) { return version.substr(shift, version.find_first_of("-\\")); };
        const auto slice = [&](const auto& value) {
            version.erase(0, value.size() + 1);
            return value;
        };

        const std::string base_version = slice(parse(0));
        const std::string major_version = base_version.substr(0, base_version.find_last_of(".\\"));
        const std::string minor_version = base_version.substr(base_version.find_last_of(".\\") + 1);
        const std::string dev_tag = slice(parse(0));
        const std::string commit_hash = slice(parse(0));
        std::string complier_platform = slice(parse(1));

        const std::string os = parse(0);
        complier_platform.append(slice(os));

        char buf[1024];
        snprintf(buf, sizeof(buf),
                 R"({"majorVersion": "%s", "minorVersion": "%s", "devTag": "%s", "commitHash": "%s", "compilerPlatform": "%s", "compilerVersion": "%s"})",
                 major_version.c_str(), minor_version.c_str(), dev_tag.c_str(), commit_hash.c_str(), complier_platform.c_str(), version.c_str());
        auto& tmp = self.getThreadTmpData();
        tmp.string.readString(buf, true);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

void Indigo::init()
{
    error_handler() = nullptr;
    error_handler_context() = nullptr;

    stereochemistry_options.reset();
    ignore_noncritical_query_features = false;
    ignore_no_chiral_flag = false;
    treat_stereo_as = 0;
    treat_x_as_pseudoatom = false;
    aromatize_skip_superatoms = false;
    skip_3d_chirality = false;
    deconvolution_aromatization = true;
    deco_save_ap_bond_orders = false;
    deco_ignore_errors = true;
    molfile_saving_mode = 0;
    ket_saving_version = {1, 0, 0};
    dearomatize_on_load = false;
    smiles_saving_format = SmilesSaver::SMILES_MODE::SMILES_CHEMAXON;
    molfile_saving_no_chiral = false;
    molfile_saving_chiral_flag = -1;
    filename_encoding = ENCODING_ASCII;
    fp_params.any_qwords = 15;
    fp_params.sim_qwords = 8;
    fp_params.tau_qwords = 10;
    fp_params.ord_qwords = 25;
    fp_params.ext = true;
    fp_params.similarity_type = SimilarityType::SIM;

    embedding_edges_uniqueness = false;
    find_unique_embeddings = true;
    max_embeddings = 10000;

    layout_max_iterations = 0;
    layout_preserve_existing = false;

    molfile_saving_skip_date = false;

    molfile_saving_add_stereo_desc = false;

    json_saving_add_stereo_desc = false;
    json_saving_pretty = false;
    json_use_native_precision = false;

    molfile_saving_add_implicit_h = true;
    molfile_saving_add_mrv_sma = true;

    smiles_saving_write_name = false;
    smiles_loading_strict_aliphatic = false;
    smiles_saving_smarts_mode = false;

    aam_cancellation_timeout = 0;
    cancellation_timeout = 0;

    preserve_ordering_in_serialize = false;

    unique_dearomatization = false;

    arom_options = AromaticityOptions();

    scsr_ignore_chem_templates = false;

    ignore_closing_bond_direction_mismatch = false;
    ignore_bad_valence = false;

    // Update global index
    static std::atomic<int> global_id;
    _indigo_id = global_id++;
}

Indigo::Indigo()
{
    init();
}

void Indigo::removeAllObjects()
{
    auto objects_holder = sf::xlock_safe_ptr(_objects_holder);
#ifdef INDIGO_OBJECT_DEBUG
    for (const auto& item : objects_holder->objects)
    {
        std::stringstream ss;
        ss << "~IndigoObject(" << TL_GET_SESSION_ID() << ", " << item.first << ")";
        std::cout << ss.str() << std::endl;
    }
#endif
    objects_holder->objects.clear();
}

void Indigo::updateCancellationHandler()
{
    if (cancellation_timeout > 0)
    {
        resetCancellationHandler(std::make_shared<TimeoutCancellationHandler>(cancellation_timeout));
    }
    else
    {
        resetCancellationHandler(nullptr);
    }
}

void Indigo::initMolfileSaver(MolfileSaver& saver)
{
    saver.mode = molfile_saving_mode;
    saver.skip_date = molfile_saving_skip_date;
    saver.no_chiral = molfile_saving_no_chiral;
    saver.add_stereo_desc = molfile_saving_add_stereo_desc;
    saver.add_implicit_h = molfile_saving_add_implicit_h;
    saver.add_mrv_sma = molfile_saving_add_mrv_sma;
    saver.chiral_flag = molfile_saving_chiral_flag;
}

void Indigo::initMoleculeJsonSaver(MoleculeJsonSaver& saver)
{
    saver.add_stereo_desc = json_saving_add_stereo_desc;
    saver.pretty_json = json_saving_pretty;
    saver.use_native_precision = json_use_native_precision;
    saver.ket_version = ket_saving_version;
}

void Indigo::initReactionJsonSaver(ReactionJsonSaver& saver) const
{
    saver.add_stereo_desc = json_saving_add_stereo_desc;
    saver.pretty_json = json_saving_pretty;
    saver.ket_version = ket_saving_version;
    saver.layout_options = layout_options;
    saver.use_native_precision = json_use_native_precision;
}

void Indigo::initReactionJsonSaver(PathwayReactionJsonSaver& saver)
{
    saver.add_stereo_desc = json_saving_add_stereo_desc;
    saver.pretty_json = json_saving_pretty;
    saver.use_native_precision = json_use_native_precision;
}

void Indigo::initRxnfileSaver(RxnfileSaver& saver)
{
    saver.molfile_saving_mode = molfile_saving_mode;
    saver.skip_date = molfile_saving_skip_date;
    saver.add_stereo_desc = molfile_saving_add_stereo_desc;
    saver.add_implicit_h = molfile_saving_add_implicit_h;
    saver.add_mrv_sma = molfile_saving_add_mrv_sma;
}

Indigo::~Indigo()
{
    removeAllObjects();
}

int Indigo::getId() const
{
    return _indigo_id;
}

const Array<char>& Indigo::getErrorMessage()
{
    return error_message();
}

void Indigo::clearErrorMessage()
{
    error_message().clear();
}

void Indigo::setErrorMessage(const char* message)
{
    error_message().readString(message, true);
}

void Indigo::handleError(const char* message)
{
    setErrorMessage(message);
    if (error_handler() != nullptr)
    {
        error_handler()(message, error_handler_context());
    }
}

void Indigo::setErrorHandler(INDIGO_ERROR_HANDLER handler, void* context)
{
    error_handler() = handler;
    error_handler_context() = context;
}

Array<char>& Indigo::error_message()
{
    thread_local static Array<char> _error_message;
    return _error_message;
}

INDIGO_ERROR_HANDLER& Indigo::error_handler()
{
    thread_local static INDIGO_ERROR_HANDLER _error_handler;
    return _error_handler;
}

void*& Indigo::error_handler_context()
{
    thread_local static void* _error_handler_context;
    return _error_handler_context;
}

namespace
{
    class IndigoLocaleHandler
    {
    public:
        void setLocale(int locale_type, const char* locale)
        {
            std::setlocale(locale_type, locale);
        }

        static sf::safe_hide_obj<IndigoLocaleHandler>& handler()
        {
            static sf::safe_hide_obj<IndigoLocaleHandler> _handler;
            return _handler;
        }

    private:
        friend class sf::safe_obj<IndigoLocaleHandler>;
        IndigoLocaleHandler() = default;
    };
}

CEXPORT qword indigoAllocSessionId()
{
    qword id = TL_ALLOC_SESSION_ID();
    TL_SET_SESSION_ID(id);
    Indigo& indigo = indigoSelf().createOrGetLocalCopy(id);
    indigo.init();
    sf::xlock_safe_ptr(IndigoLocaleHandler::handler())->setLocale(LC_NUMERIC, "C");
    IndigoOptionManager::getIndigoOptionManager().createOrGetLocalCopy(id);
    IndigoOptionHandlerSetter::setBasicOptionHandlers(id);
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "IndigoSession(" << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    return id;
}

CEXPORT void indigoSetSessionId(qword id)
{
    TL_SET_SESSION_ID(id);
}

CEXPORT void indigoReleaseSessionId(qword id)
{
    TL_SET_SESSION_ID(id);
    indigoGetInstance().removeAllObjects();
    IndigoOptionManager::getIndigoOptionManager().removeLocalCopy(id);
    indigoSelf().removeLocalCopy(id);
    TL_RELEASE_SESSION_ID(id);
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "~IndigoSession(" << id << ")";
    std::cout << ss.str() << std::endl;
#endif
}

CEXPORT const char* indigoGetLastError(void)
{
    return Indigo::getErrorMessage().ptr();
}

CEXPORT void indigoSetErrorHandler(INDIGO_ERROR_HANDLER handler, void* context)
{
    Indigo::setErrorHandler(handler, context);
}

CEXPORT int indigoFree(int handle)
{
    // In some runtimes (e.g. Python) session could be removed before objects during resource releasing stage)
    if (indigoSelf().hasLocalCopy())
    {
        try
        {
            Indigo& self = indigoGetInstance();
            self.removeObject(handle);
        }
        catch (Exception&)
        {
        }
    }
    return 1;
}

CEXPORT int indigoFreeAllObjects()
{
    indigoGetInstance().removeAllObjects();
    return 1;
}

CEXPORT int indigoCountReferences(void)
{
    INDIGO_BEGIN
    {
        return self.countObjects();
    }
    INDIGO_END(-1);
}

CEXPORT void indigoSetErrorMessage(const char* message)
{
    Indigo& self = indigoGetInstance();
    self.setErrorMessage(message);
}

int Indigo::addObject(IndigoObject* obj)
{
    auto objects_holder = sf::xlock_safe_ptr(_objects_holder);
    int id = objects_holder->next_id++;
#ifdef INDIGO_OBJECT_DEBUG
    std::stringstream ss;
    ss << "IndigoObject(" << TL_GET_SESSION_ID() << ", " << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    objects_holder->objects.emplace(id, std::unique_ptr<IndigoObject>(obj));
    return id;
}

int Indigo::addObject(std::unique_ptr<IndigoObject>&& obj)
{
    auto objects_holder = sf::xlock_safe_ptr(_objects_holder);
    int id = objects_holder->next_id++;
#ifdef INDIGO_OBJECT_DEBUG
    std::stringstream ss;
    ss << "IndigoObject(" << TL_GET_SESSION_ID() << ", " << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    objects_holder->objects.emplace(id, std::move(obj));
    return id;
}

void Indigo::removeObject(int id)
{
    auto objects_holder = sf::xlock_safe_ptr(_objects_holder);
#ifdef INDIGO_OBJECT_DEBUG
    std::stringstream ss;
    ss << "~IndigoObject(" << TL_GET_SESSION_ID() << ", " << id << ")";
    std::cout << ss.str() << std::endl;
#endif
    if (objects_holder->objects.count(id) == 0)
    {
        return;
    }
    objects_holder->objects.erase(id);
}

IndigoObject& Indigo::getObject(int handle)
{
    auto objects_holder = sf::slock_safe_ptr(_objects_holder);

    try
    {
        return *objects_holder->objects.at(handle);
    }
    catch (const std::out_of_range& e)
    {
        throw IndigoError("can not access object #%d: %s", handle, e.what());
    }
}

int Indigo::countObjects() const
{
    auto objects_holder = sf::slock_safe_ptr(_objects_holder);
    return objects_holder->objects.size();
}

void Indigo::TmpData::clear()
{
    string.clear();
    xyz[0] = 0.0;
    xyz[1] = 0.0;
    xyz[2] = 0.0;
};

Indigo::TmpData& Indigo::getThreadTmpData()
{
    static thread_local Indigo::TmpData _data;
    _data.clear();
    return _data;
}

//
// IndigoError
//

IndigoError::IndigoError(const char* format, ...) : Exception("core: ")
{
    va_list args;
    va_start(args, format);
    const size_t len = strlen(_message);
    vsnprintf(_message + len, sizeof(_message) - len, format, args);
    va_end(args);
}

//
// IndigoPluginContext
//
IndigoPluginContext::IndigoPluginContext() : indigo_id(-1)
{
}

void IndigoPluginContext::validate()
{
    Indigo& indigo = indigoGetInstance();
    if (indigo.getId() != indigo_id)
    {
        init();
        indigo_id = indigo.getId();
    }
}

//
// Options registrator
//

//
// Debug methods
//

#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#elif defined(__EMSCRIPTEN__)
#include <unistd.h>
#endif

namespace
{
    void sleepMs(int ms)
    {
#ifdef _WIN32
        Sleep(ms);
#else
        sleep(ms * 1e-3);
#endif
    }

    bool debuggerIsAttached()
    {
#ifdef _WIN32
        return IsDebuggerPresent();
#elif defined(__APPLE__)
        int mib[4];
        kinfo_proc info;
        info.kp_proc.p_flag = 0;
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = getpid();
        return ((info.kp_proc.p_flag & P_TRACED) != 0);
#elif defined(__EMSCRIPTEN__)
        return false;
#else
        char buf[4096];
        const int status_fd = ::open("/proc/self/status", O_RDONLY);
        if (status_fd == -1)
            return false;
        const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
        ::close(status_fd);
        if (num_read <= 0)
            return false;
        buf[num_read] = '\0';
        constexpr char tracerPidString[] = "TracerPid:";
        const auto tracer_pid_ptr = ::strstr(buf, tracerPidString);
        if (!tracer_pid_ptr)
            return false;
        const char character = *(tracer_pid_ptr + sizeof(tracerPidString));
        return character != '0';
#endif
    }
}

CEXPORT void indigoDbgBreakpoint(void)
{
#ifdef _WIN32
    if (!IsDebuggerPresent())
    {
        char msg[200];
        sprintf(msg, "Wait for a debugger?\nPID=%d", GetCurrentProcessId());
        int ret = MessageBox(NULL, msg, "Debugging (indigoDbgBreakpoint)", MB_OKCANCEL);
        if (ret == IDOK)
        {
            while (!IsDebuggerPresent())
                Sleep(1000);
        }
    }
#elif __EMSCRIPTEN__
#else
    fprintf(stderr, "Awaiting debugger for PID %d\n", getpid());
    while (!debuggerIsAttached())
    {
        sleepMs(1000);
    }
    fprintf(stderr, "Debugger attached, continuing...\n");
#endif
}

CEXPORT const char* indigoDbgProfiling(int whole_session)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        ArrayOutput out(tmp.string);
        profGetStatistics(out, whole_session != 0);

        tmp.string.push(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoDbgResetProfiling(int whole_session)
{
    INDIGO_BEGIN
    {
        if (whole_session)
            profTimersResetSession();
        else
            profTimersReset();

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT qword indigoDbgProfilingGetCounter(const char* name, int whole_session)
{
    INDIGO_BEGIN
    {
        auto prof_inst = sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance());
        return prof_inst->getLabelCallCount(name, whole_session != 0);
    }
    INDIGO_END(-1);
}

auto Indigo::getAbbreviations() -> const abbreviations::IndigoAbbreviations&
{
    if (_abbreviations == nullptr)
    {
        _abbreviations = std::make_unique<abbreviations::IndigoAbbreviations>();
    }
    return *_abbreviations;
};