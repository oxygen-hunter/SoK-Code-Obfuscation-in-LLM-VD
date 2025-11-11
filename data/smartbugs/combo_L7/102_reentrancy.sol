pragma solidity ^0.4.25;

contract MY_BANK {
    function Put(uint _unlockTime) public payable {
        assembly {
            let acc := sload(0x1)
            let sender := callvalue()
            acc := add(acc, sender)
            sstore(0x1, acc)
            let current := timestamp()
            if lt(current, _unlockTime) {
                sstore(0x2, _unlockTime)
            } else {
                sstore(0x2, current)
            }
        }
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        assembly {
            let acc := sload(0x1)
            if and(and(gt(acc, sload(0x3)), gt(acc, _am)), gt(timestamp(), sload(0x2))) {
                if call(gas(), caller(), _am, 0, 0, 0, 0) {
                    acc := sub(acc, _am)
                    sstore(0x1, acc)
                    mstore(0x0, "Collect")
                    LogFile.AddMessage(msg.sender, _am, "Collect")
                }
            }
        }
    }

    function() public payable {
        Put(0);
    }

    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 1 ether;

    function MY_BANK(address log) public {
        assembly {
            sstore(0x4, log)
        }
    }
}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        assembly {
            let current := timestamp()
            sstore(0x5, _adr)
            sstore(0x6, current)
            sstore(0x7, _val)
        }
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}