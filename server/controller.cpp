/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#include "controller.h"

#include <assert.h>

#include <thread>

#include "acceptor.h"
#include "io_handler.h"
#include "worker.h"
#include "message_center.h"
#include "global_configure.h"
#include "common/logger.h"
#include "common/macro.h"

ef::GlobalConfigure gGlobalConfigure;

namespace ef {

#define DELETE_CONTAINER(container) do {    \
    for (auto & it : container) {           \
        DELETE_POINTER(it);                 \
    }                                       \
} while (0)

#define CONTAINER_DO(container, todo) do {  \
    for (auto & it : container) {           \
        it->todo();                         \
    }                                       \
} while (0)

Controller::Controller()
    : _acceptor_id_generator(0)
    , _iohandler_id_generator(0)
    , _worker_id_generator(0)
{
}

Controller::~Controller()
{
    StopServer();

    DELETE_CONTAINER(_acceptors);
    DELETE_CONTAINER(_iohandlers);
    DELETE_CONTAINER(_workers);
    DELETE_CONTAINER(_acceptor_threads);
    DELETE_CONTAINER(_iohandler_threads);
    DELETE_CONTAINER(_worker_threads);
}

bool Controller::InitServer()
{
    // if (!ef::Logger::Initialize("server", 1000, 5, ef::kLevelInfo))
    // {
    //     printf("Initialize failed, errmsg: %s\n", ef::Logger::GetErrMsg().c_str());
    //     return -1;
    // }

    // ef::Logger::SetLogLevel(gGlobalConfigure.log_level);
    ef::Logger::SetLogLevel(kLevelErr);

    // TODO
    if (!InitAcceptor() || !InitIoHandler() || !InitWorker()) {
        return false;
    }
    return true;
}

void Controller::StartServer()
{
    // start acceptor
    for (auto &it : _acceptors) {
        std::thread * t = new std::thread(&Acceptor::Run, it);
        assert(t);
        _acceptor_threads.push_back(t);
    }

    // start iohandler
    for (auto &it : _iohandlers) {
        std::thread * t = new std::thread(&IoHandler::Run, it);
        assert(t);
        _iohandler_threads.push_back(t);
    }

    // start worker
    for (auto &it : _workers) {
        std::thread * t = new std::thread(&Worker::Run, it);
        assert(t);
        _worker_threads.push_back(t);
    }
}

void Controller::StopServer()
{
    // 如果初始化失败,则这块不相等
    // assert(_acceptors.size() == _acceptor_threads.size());
    // assert(_iohandlers.size() == _iohandler_threads.size());
    // assert(_workers.size() == _worker_threads.size());

    CONTAINER_DO(_acceptors, Stop);
    CONTAINER_DO(_iohandlers, Stop);
    CONTAINER_DO(_workers, Stop);

    CONTAINER_DO(_acceptor_threads, join);
    CONTAINER_DO(_iohandler_threads, join);
    CONTAINER_DO(_worker_threads, join);
}

bool Controller::InitAcceptor()
{
    std::vector<unsigned short> listen_ports = GetListenPorts();
    for (int i=0; i<gGlobalConfigure.acceptor_count; ++i) {
        Acceptor * acceptor = new Acceptor(listen_ports.data(), listen_ports.size());
        assert(acceptor);
        _acceptors.push_back(acceptor);
        MessageCenter::Register(acceptor);
        if (!acceptor->Initialize(_acceptor_id_generator++)) {
            _errmsg = acceptor->GetErrMsg();
            return false;
        }
    }
    return true;
}

bool Controller::InitIoHandler()
{
    for (int i=0; i<gGlobalConfigure.iohandler_count; ++i) {
        IoHandler * iohandler = new IoHandler();
        assert(iohandler);
        _iohandlers.push_back(iohandler);
        MessageCenter::Register(iohandler);
        if (!iohandler->Initialize(_iohandler_id_generator++)) {
            _errmsg = iohandler->GetErrMsg();
            return false;
        }
    }
    return true;
}

bool Controller::InitWorker()
{
    for (int i=0; i<gGlobalConfigure.woker_count; ++i) {
        Worker * worker = new Worker();
        assert(worker);
        _workers.push_back(worker);
        MessageCenter::Register(worker);
        if (!worker->Initialize(_worker_id_generator++)) {
            _errmsg = worker->GetErrMsg();
            return false;
        }
    }
    return true;
}

std::vector<unsigned short> Controller::GetListenPorts()
{
    // TODO
    std::vector<unsigned short> vec;
    vec.push_back(12345);
    vec.push_back(12355);
    return std::move(vec);
}

const std::string Controller::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
