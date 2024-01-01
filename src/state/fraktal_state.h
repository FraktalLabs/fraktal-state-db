#pragma once

#include <iostream>

#include "fraktal_account.h"

#include <evm-cpp-utils/types.h>

#define FRAKTAL_VM_MUTEX_COUNT 1024

class FraktalState : public State {
public:
  FraktalState() = default;
  FraktalState(const std::string& snapshot): State() {
    fraktalRestore(snapshot);
  }

  void fraktalSnapshot(const std::string& snapshot);
  virtual void snapshot(const std::string& snapshot) override { fraktalSnapshot(snapshot); }
  void fraktalRestore(const std::string& snapshot);
  virtual void restore(const std::string& snapshot) override { fraktalRestore(snapshot); }

  void lockMutex(uint64_t mutexId);
  void unlockMutex(uint64_t mutexId);
  virtual void insert(const address& addr, std::shared_ptr<Account> account) override;

private:
  std::array<std::mutex, FRAKTAL_VM_MUTEX_COUNT> mutexSpace;
  std::array<uint64_t, FRAKTAL_VM_MUTEX_COUNT> mutexNonces;
};
