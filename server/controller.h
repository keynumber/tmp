/*
 * Author: number
 * Date  : Feb 9, 2016
 */

#ifndef __SERVER_CONTROLLER_H_H___
#define __SERVER_CONTROLLER_H_H___

#include <stdint.h>

#include <vector>
#include <string>
#include <thread>

namespace ef {

class Acceptor;
class IoHandler; class Worker;


/**
 * 这样实现,server还可以短暂停止,不会有任何影响
 * 启动(start),暂停(suspend),恢复(resume),重启(restart),停止(stop)
 */
class Controller {
public:
    Controller();
    virtual ~Controller();

    bool InitServer();
    void StartServer();     // 启动运行,如果server处于启动状态,则相当于重启
    void StopServer();      // 暂停运行,可以通过StartServer重新启动,暂停前所有的状态/数据都不会丢失
                            // 如果想要终止server,可以通过析构Controller进行

    void RegisterHandler();

    // TODO below
    bool AddAcceptor();
    bool AddIoHandler();
    bool AddWorker();
    bool StopOneAcceptor();
    bool StopOneIoHandler();
    bool StopOneWorker();
    bool ReportServerStatus() const;
    // TODO upon

    const std::string GetErrMsg() const;

private:
    bool InitAcceptor();
    bool InitIoHandler();
    bool InitWorker();

    std::vector<unsigned short> GetListenPorts();

private:
    std::vector<Acceptor *> _acceptors;
    std::vector<std::thread*> _acceptor_threads;
    std::vector<IoHandler *> _iohandlers;
    std::vector<std::thread*> _iohandler_threads;
    std::vector<Worker *> _workers;
    std::vector<std::thread*> _worker_threads;

    uint32_t _acceptor_id_generator;
    uint32_t _iohandler_id_generator;
    uint32_t _worker_id_generator;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __SERVER_CONTROLLER_H__ */
