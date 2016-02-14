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

class Controller {
public:
    Controller();
    virtual ~Controller();

    bool InitServer();
    void StartServer();
    void StopServer();

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
