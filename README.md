# Fraktal State DB

[![readme style standard](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

Fraktal VM state database

Stand-alone C++20 executable and library used for Fraktal VM state.
Fork of ![evm-state-db][evm-state-db] with state & account class overrides.
State access done thru nonce-locks to enable replayablity & thread safety.

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Dependencies](#dependencies)
- [Testing](#testing)
- [Details](#details)
- [Media](#media)
- [Maintainer](#maintainer)
- [License](#license)

## Install

```
make all
```

**NOTE:** [Dependencies](#dependencies) must be compiled & linked into compilation aswell.

## Usage

fraktal-state-db comes with various commandline functions, including :

**Get** - get a value at (contract address, key) pair in state
```
./bin/fraktal-state-db get --snapshotFile ./path/to/snapshot.json --contractAddress 4200000000000000000000000000000000000000 --key 42
```

**Set** - set a value at (contract address, key) pair in state ( and saves changes )
```
./bin/fraktal-state-db set --snapshotFile ./path/to/snapshot.json --contractAddress 4200000000000000000000000000000000000000 --key 42 --value 512
```

**Run** - runs an RPC server giving access to Fraktal VM state db ( for gets and sets )
( TODO: not yet implemented )
```
./bin/fraktal-state-db run --snapshotFile ./path/to/snapshot.json --rpcPort 8999
# Get request
curl -X POST -H "Content-Type: application/json" --data '--contractAddress 4200000000000000000000000000000000000000 --key 42' http://localhost:8999
# Set request
curl -X POST -H "Content-Type: application/json" --data '--contractAddress 4200000000000000000000000000000000000000 --key 42 --value 512' http://localhost:8999
```

**Note** : Use `-h` flag on each sub-command to see more options

## Dependencies

- [intx][intx] : 256-bit (32-byte) unsigned integers
- [ethash][ethash] : Ethereum hash functions
- [evm-cpp-utils][evm-cpp-utils] : EVM state types + utils

## Testing

This repo contains various tests under `./test/` to make sure things are working.
To run use :

```
make get-test
make set-test

# RPC test
make run-rpc-local
# In a seperate terminal
make get-rpc-test
make set-rpc-test
make get-rpc-test
# Ctrl-C to stop RPC & snapshot state
```

Check the diff in `./test/snapshot.json` to see if things processed properly.

## Details

Fraktal VM allows parallel execution of smart contracts.
Thus, there are potential race conditions on state access operations.

In order to prevent race conditions on set operations :
1. `FraktalState` provides a `mutexSpace`, a fixed size array of mutexes for accessing state.
2. When `FraktalAccount` does a `setStorage` operation,
    lock the mutex at position `hash(contract addr, storage key) % mutexSpaceSize`

In order to prevent race conditions on get operations :
1. `FraktalState` provides `mutexNonces`, nonce that increments when corresponding mutex locks.
2. `FraktalAccount` provides `storageNonces`, nonce that increments when storage value is set.
3. `FraktalAccount` provides `storageNonceValueHistory`, prunable 2d map from key -> nonce -> value
4. When `FraktalAccount` does a `getStorage` operation,
    get the `storageNonce` at key then get the value at the corresponding `(key, storage nonce)` pair in history

In order to allow custom locking operations for things like sudo-atomic operations ( Not implemented ) :
1. `FraktalAccount` provides `mutexStorage`, a dynamic mutex storage set

Fraktal State DB provides extra functionality for snapshotting new nonces & history.

**DA** ( Not implemented ) : Fraktal State DB provides mutex, nonce, and value history data to DA agent(s).

## Media

![Fraktal State DB](https://github.com/FraktalLabs/docs/blob/master/images/fraktal-state-db/fraktal-state-db.jpg)

## Maintainer

Brandon Roberts [@b-j-roberts]

## License

[MIT][MIT]


[intx]: https://github.com/chfast/intx
[ethash]: http://github.com/chfast/ethash
[evm-cpp-utils]: https://github.com/FraktalLabs/evm-cpp-utils
[evm-state-db]: git@github.com:FraktalLabs/evm-state-db.git
[MIT]: LICENSE
[@b-j-roberts]: https://github.com/b-j-roberts
