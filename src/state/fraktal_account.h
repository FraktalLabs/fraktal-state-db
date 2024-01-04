#pragma once

#include <iostream>

#include "fraktal_state.h"

#include <evm-cpp-utils/types.h>

class FraktalState;

class FraktalAccount : public Account {
public:
    // TODO: Disable these?
    FraktalAccount() = default;
    FraktalAccount(const std::string& serialized) : Account() { fraktalAccountFromSerialized(serialized); }
    FraktalAccount(const Account& account) : Account(account) {}

    FraktalAccount(FraktalState* state, const address& accountAddress) : Account(), state(state), accountAddress(accountAddress) {}
    FraktalAccount(FraktalState* state, const address& accountAddress, const std::string& serialized) :
      Account(), state(state), accountAddress(accountAddress) { fraktalAccountFromSerialized(serialized); }
    FraktalAccount(const Account& account, FraktalState* state, const address& accountAddress) :
      Account(account), state(state), accountAddress(accountAddress) {}
    FraktalAccount(std::shared_ptr<Account> account, FraktalState* state, const address& accountAddress) :
      Account(account), state(state), accountAddress(accountAddress) {}

    std::string fraktalAccountToSerialized() const;
    std::string toString() const override { return fraktalAccountToSerialized(); }
    virtual std::string toSerialized() const override { return fraktalAccountToSerialized(); }
    void fraktalAccountFromSerialized(const std::string& serialized);
    virtual void fromSerialized(const std::string& serialized) override { fraktalAccountFromSerialized(serialized); }

    // TODO : set nonce, other fields?
    
    virtual void setStorage(const intx::uint256& key, const intx::uint256& value) override;
    intx::uint256 getStorageAtNonce(const intx::uint256& key, const intx::uint256& nonce) const;
    virtual intx::uint256 getStorage(const intx::uint256& key) const override;

    // TODO: Provide non mutex / history regarding set and get functions for testing, ...

private:
    // TODO: account only mutex or state mutex? w/ both having seperate nonce?
    FraktalState* state;
    address accountAddress;
    std::map<intx::uint256, intx::uint256> storageNonces;

    std::map<intx::uint256, std::map<intx::uint256, intx::uint256>> storageNonceValueHistory;
};
