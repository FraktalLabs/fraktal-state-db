#include "fraktal_state.h"
#include "fraktal_account.h"

void FraktalState::fraktalSnapshot(const std::string& filepath) {
  // For now : Use pretty-json like formatting
  std::string serialized = "{\n\"accounts\": [\n";
  for (auto it = state.begin(); it != state.end(); ++it) {
    serialized += "  {\n";
    serialized += "    \"";
    for (const auto &byte : it->first) {
      serialized += byteToHex(byte);
    }
    serialized += "\" : " + it->second->toSerialized();
    if (it == --state.end()) {
      serialized += "\n  }\n";
    } else {
      serialized += "\n  },\n";
    }
  }
  serialized += "],\n";
  serialized += "\"mutexesNonces\": [\n";
  for (int i = 0; i < FRAKTAL_VM_MUTEX_COUNT; i++) {
    serialized += "    \"" + std::to_string(i) + "\": " + std::to_string(mutexNonces[i]);
    if (i == FRAKTAL_VM_MUTEX_COUNT - 1) {
      serialized += "\n";
    } else {
      serialized += ",\n";
    }
  }
  serialized += "  ]\n";
  serialized += "}\n";

  // Open file for writing
  std::ofstream file(filepath);
  file << serialized;
  file.close();
}

void FraktalState::fraktalRestore(const std::string& filepath) {
  // Open file for reading
  std::ifstream file(filepath);
  std::string line;

  // Flatten json file
  std::string flattened;
  while (std::getline(file, line)) {
    // trim line
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
    flattened += line;
  }
  file.close();

  // Parse flattened json
  // { "accounts": [ { "address": { value } }, { "address": { value } } ], "mutexesNonces": [ "0": 0, "1": 0, ... ] }

  // Get accounts array & mutexesNonces array from flattened json
  std::string accounts;
  std::string mutexesNonces;
  bool isAccounts = false;
  bool accountsDone = false;
  bool isMutexesNonces = false;
  uint64_t bracketCount = 0;
  for (const auto &c : flattened) {
    if (c == '[') {
      if (accountsDone) {
        isMutexesNonces = true;
      } else {
        isAccounts = true;
      }
      bracketCount++;
      if(bracketCount != 1) {
        if (isAccounts) {
          accounts += c;
        } else if (isMutexesNonces) {
          mutexesNonces += c;
        }
      }
    } else if (c == ']') {
      bracketCount--;
      if (bracketCount == 0) {
        if (accountsDone) {
          isMutexesNonces = false;
          break;
        } else {
          isAccounts = false;
          accountsDone = true;
        }
      } else {
        if (isAccounts) {
          accounts += c;
        } else if (isMutexesNonces) {
          mutexesNonces += c;
        }
      }
    } else if (bracketCount > 0) {
      if (isAccounts) {
        accounts += c;
      } else if (isMutexesNonces) {
        mutexesNonces += c;
      }
    }
  }

  // Loop through { "address": { value } } items
  std::string item;
  bool isItem = false;
  bracketCount = 0;
  for (const auto &c : accounts) {
    if (c == '{') {
      isItem = true;
      bracketCount++;
      if (bracketCount != 1) {
        item += c;
      }
    } else if (c == '}') {
      bracketCount--;
      if (bracketCount == 0) {
        isItem = false;

        std::string key;
        std::string value;
        bool isKey = false;
        bool isValue = false;
        bool valueColon = false;
        for (const auto &c : item) {
          if (c == '"' && !isValue) {
            if (isKey) {
              isKey = false;
              isValue = true;
              valueColon = true;
            } else {
              isKey = true;
            }
          } else if (c == ':' && !isValue) {
            continue;
          } else if (c == ':' && valueColon) {
            valueColon = false;
            continue;
          } else if (isKey) {
            key += c;
          } else if (isValue) {
            value += c;
          }
        }
        uint8_t keyBytes[20];
        address keyAddr;
        for (int i = 0; i < 20; i++) {
          std::string byteStr = key.substr(i * 2, 2);
          keyBytes[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
        }
        address keyArray;
        std::copy(keyBytes, keyBytes + 20, keyArray.begin());
        std::shared_ptr<FraktalAccount> fraktalAccount = std::make_shared<FraktalAccount>(this, keyArray, value);
        State::insert(keyArray, std::static_pointer_cast<Account>(fraktalAccount));
        // TODO: insert(keyArray, std::dynamic_pointer_cast<Account>(fraktalAccount));
        //state[keyBytes] = Account(value);

        item = "";
      } else {
        item += c;
      }
    } else if (isItem) {
      item += c;
    }
  }

  // Loop through "0": 0, "1": 0, ... items
  std::string key;
  std::string value;
  bool isKey = false;
  for (const auto &c : mutexesNonces) {
    if (c == '"') {
      if (isKey) {
        isKey = false;
      } else {
        isKey = true;
      }
    } else if (c == ':') {
      continue;
    } else if (isKey) {
      key += c;
    } else {
      value += c;
    }
    if (c == ',') {
      uint64_t mutexIndex = std::stoi(key);
      uint64_t mutexNonce = std::stoi(value);
      mutexNonces[mutexIndex] = mutexNonce;
      key = "";
      value = "";
    }
  }
  // Last key-value pair
  if (key != "" && value != "") {
    uint64_t mutexIndex = std::stoi(key);
    uint64_t mutexNonce = std::stoi(value);
    mutexNonces[mutexIndex] = mutexNonce;
  }
}

void FraktalState::lockMutex(uint64_t mutexId) {
  uint64_t mutexIndex = mutexId % FRAKTAL_VM_MUTEX_COUNT;
  mutexSpace[mutexIndex].lock();
  mutexNonces[mutexIndex] += 1;
}

void FraktalState::unlockMutex(uint64_t mutexId) {
  mutexSpace[mutexId % FRAKTAL_VM_MUTEX_COUNT].unlock();
}

void FraktalState::insert(const address& addr, std::shared_ptr<Account> account) {
  std::shared_ptr<FraktalAccount> fraktalAccount = std::make_shared<FraktalAccount>(account, this, addr);

  State::insert(addr, std::static_pointer_cast<Account>(fraktalAccount));
}
