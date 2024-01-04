#include "fraktal_account.h"

#include <ethash/keccak.h>

// TODO: Make sure all locks are locked / stopped before snapshotting

std::string FraktalAccount::fraktalAccountToSerialized() const {
  // For now : Use pretty-json like formatting
  std::string serialized = "{\n";
  serialized += "      \"nonce\": " + std::to_string(nonce) + ",\n";
  serialized += "      \"balance\": " + to_string(balance) + ",\n";
  serialized += "      \"storage\": [\n";
  for (auto it = storage.begin(); it != storage.end(); ++it) {
    serialized += "        \"" + to_string(it->first) + "\": " + to_string(it->second) + "";
    if (std::next(it) != storage.end()) {
      serialized += ",";
    }
    serialized += "\n";
  }
  serialized += "      ],\n";
  serialized += "      \"storage-nonces\": [\n";
  for (auto it = storageNonces.begin(); it != storageNonces.end(); ++it) {
    serialized += "        \"" + to_string(it->first) + "\": " + to_string(it->second) + "";
    if (std::next(it) != storageNonces.end()) {
      serialized += ",";
    }
    serialized += "\n";
  }
  serialized += "      ],\n";
  serialized += "      \"storage-nonce-value-history\": {\n";
  for (auto it = storageNonceValueHistory.begin(); it != storageNonceValueHistory.end(); ++it) {
    serialized += "        \"" + to_string(it->first) + "\": [\n";
    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      serialized += "          \"" + to_string(it2->first) + "\": " + to_string(it2->second) + "";
      if (std::next(it2) != it->second.end()) {
        serialized += ",";
      }
      serialized += "\n";
    }
    serialized += "        ]";
    if (std::next(it) != storageNonceValueHistory.end()) {
      serialized += ",";
    }
    serialized += "\n";
  }
  serialized += "      },\n";
  serialized += "      \"code\": \"";
  for (const auto &byte : code) {
    serialized += byteToHex(byte);
  }
  serialized += "\"\n";
  serialized += "    }";
  return serialized;
}

void FraktalAccount::fraktalAccountFromSerialized(const std::string& serialized) {
  // Parse the flattened json like
  // {"nonce"123,"balance"3001,"storage"[],"code""6020600052"}
  size_t i = 0;
  for (; i < serialized.size(); ++i) {
    if (serialized[i] == '{') {
      continue;
    } else if (serialized[i] == '"') {
      // Parse key
      std::string key;
      for (++i; serialized[i] != '"'; ++i) {
        key += serialized[i];
      }
      ++i; // Skip the closing "
      ++i; // Skip the :
      if (key == "nonce") {
        // Parse nonce
        std::string nonceStr;
        for (; serialized[i] != ','; ++i) {
          nonceStr += serialized[i];
        }
        nonce = std::stoull(nonceStr);
      } else if (key == "balance") {
        // Parse balance
        std::string balanceStr;
        for (; serialized[i] != ','; ++i) {
          balanceStr += serialized[i];
        }
        balance = intx::from_string<intx::uint256>(balanceStr);
      } else if (key == "storage") {
        // Parse storage
        for (; serialized[i] != '['; ++i) {}
        for (++i; serialized[i] != ']'; ++i) {
          // Parse key
          std::string keyStr;
          for (++i; serialized[i] != '"'; ++i) {
            keyStr += serialized[i];  
          }
          ++i; // Skip the closing "
          ++i; // Skip the :
          // Parse value
          std::string valueStr;
          for (; serialized[i] != ',' && serialized[i] != ']'; ++i) {
            valueStr += serialized[i];
          }
          storage[intx::from_string<intx::uint256>(keyStr)] = intx::from_string<intx::uint256>(valueStr);
          if (serialized[i] == ']') {  
            break;
          }
        }
      } else if (key == "storage-nonces") {
        // Parse storage nonces
        for (; serialized[i] != '['; ++i) {}
        for (++i; serialized[i] != ']'; ++i) {
          // Parse key
          std::string keyStr;
          for (++i; serialized[i] != '"'; ++i) {
            keyStr += serialized[i];  
          }
          ++i; // Skip the closing "
          ++i; // Skip the :
          // Parse value
          std::string valueStr;
          for (; serialized[i] != ',' && serialized[i] != ']'; ++i) {
            valueStr += serialized[i];
          }
          storageNonces[intx::from_string<intx::uint256>(keyStr)] = intx::from_string<intx::uint256>(valueStr);
          if (serialized[i] == ']') {  
            break;
          }
        }
      } else if (key == "storage-nonce-value-history") {
        // Parse storage nonce value history
        for (; serialized[i] != '{'; ++i) {}
        for (++i; serialized[i] != '}'; ++i) {
          // Parse key
          std::string keyStr;
          for (++i; serialized[i] != '"'; ++i) {
            keyStr += serialized[i];  
          }
          ++i; // Skip the closing "
          ++i; // Skip the :
          // Parse value
          for (; serialized[i] != '['; ++i) {}
          for (++i; serialized[i] != ']'; ++i) {
            // Parse key
            std::string keyStr2;
            for (++i; serialized[i] != '"'; ++i) {
              keyStr2 += serialized[i];  
            }
            ++i; // Skip the closing "
            ++i; // Skip the :
            // Parse value
            std::string valueStr;
            for (; serialized[i] != ',' && serialized[i] != ']'; ++i) {
              valueStr += serialized[i];
            }
            storageNonceValueHistory[intx::from_string<intx::uint256>(keyStr)][intx::from_string<intx::uint256>(keyStr2)] = intx::from_string<intx::uint256>(valueStr);
            if (serialized[i] == ']') {  
              break;
            }
          }
          for(; serialized[i] != ',' && serialized[i] != '}'; ++i) {}
          if (serialized[i] == '}') {  
            break;
          }
        }
      } else if (key == "code") {
        // Parse code
        ++i; // Skip the opening "
        for (; serialized[i] != '"'; ++i) {
          uint8_t byte = (uint8_t) strtol(serialized.substr(i, 2).c_str(), nullptr, 16);
          code.push_back(byte);
          ++i;
        }
      }
    } 
  }
}
    
void FraktalAccount::setStorage(const intx::uint256& key, const intx::uint256& value) {
  // Mutex hash fields : accountAddress, key pair // TODO: should I include any more fields like nonce?
  std::string dataString = addressToHex(accountAddress) + intx::to_string(key) + intx::to_string(storageNonces[key]);
  uint8_t* dataPtr = (uint8_t*)dataString.c_str();
  intx::uint256 mutexId = intx::be::load<intx::uint256>(ethash_keccak256(dataPtr, dataString.size()));

  // Set storage value w/ state mutex
  state->lockMutex(static_cast<uint64_t>(mutexId));
  storageNonces[key] += 1;
  Account::setStorage(key, value);
  storageNonceValueHistory[key][storageNonces[key]] = Account::getStorage(key);
  // TODO: Clear out history after DA done? ( so use a queue? )
  state->unlockMutex(static_cast<uint64_t>(mutexId));
}

intx::uint256 FraktalAccount::getStorageAtNonce(const intx::uint256& key, const intx::uint256& nonce) const {
  auto storageKeyHistoryIt = storageNonceValueHistory.find(key);
  if (storageKeyHistoryIt != storageNonceValueHistory.end()) {
    auto storageNonceValueHistoryIt = storageKeyHistoryIt->second.find(nonce);
    if (storageNonceValueHistoryIt != storageKeyHistoryIt->second.end()) {
      return storageNonceValueHistoryIt->second;
    } else {
      // Go back one nonce & try again
      if (nonce > 0) {
        return getStorageAtNonce(key, nonce-1);
      } else {
        return 0;
      }
    }
  } else {
    return 0;
  }
}

intx::uint256 FraktalAccount::getStorage(const intx::uint256& key) const {
  // TODO: 
  auto storageNonceIt = storageNonces.find(key);
  intx::uint256 storageNonce = (storageNonceIt != storageNonces.end() ? storageNonceIt->second : 0);

  return getStorageAtNonce(key, storageNonce);
  //intx::uint256 storageNonce = (storageNonces.find(key) != storageNonces.end() ? storageNonces.at(key) : 0);
  //bool storageHistoryKeyExists = (storageNonceValueHistory.find(key) != storageNonceValueHistory.end());
  //bool storageNonceValueInHistory = (storageHistoryKeyExists &&
  //    storageNonceValueHistory.at(key).find(storageNonce) != storageNonceValueHistory.at(key).end());
  //if (storageNonceValueInHistory) {
  //  return storageNonceValueHistory.at(key).at(storageNonce);
  //} else {
  //  if (storageHistoryKeyExists) {
  //    return storageNonceValueHistory.at(key).at(storageNonce-1);
  //  } else {
  //    return 0;
  //  }
  //}
  // TODO: provide DA on nonce, value pairs used in gets
}
