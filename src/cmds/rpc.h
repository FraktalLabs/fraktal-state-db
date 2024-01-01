#pragma once

#include <string>
#include <memory>

#include "../state/fraktal_state.h"

class RpcServer {
public:
    RpcServer(std::shared_ptr<FraktalState> state): state(state) {}

    void Start(const std::string& addr, int port);
    int Exec(const std::string& cmd);
private:
    std::shared_ptr<FraktalState> state;
};
