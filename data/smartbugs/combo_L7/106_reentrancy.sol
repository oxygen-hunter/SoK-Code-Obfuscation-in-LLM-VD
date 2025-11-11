pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT {
    mapping (address => uint256) public balances;   
    
    uint public MinSum = 1 ether;
    
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool intitalized;
    
    function SetMinSum(uint _val) public {
        assembly {
            let isInit := sload(intitalized_slot)
            if iszero(isInit) {
                sstore(MinSum_slot, _val)
            } else {
                revert(0, 0)
            }
        }
    }
    
    function SetLogFile(address _log) public {
        assembly {
            let isInit := sload(intitalized_slot)
            if iszero(isInit) {
                sstore(Log_slot, _log)
            } else {
                revert(0, 0)
            }
        }
    }
    
    function Initialized() public {
        assembly {
            sstore(intitalized_slot, 1)
        }
    }
    
    function Deposit() public payable {
        balances[msg.sender] += msg.value;
        Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am) public payable {
        assembly {
            let minSum := sload(MinSum_slot)
            let balance := sload(add(balances_slot, caller))
            if and(ge(balance, minSum), ge(balance, _am)) {
                let success := call(gas, caller, _am, 0, 0, 0, 0)
                if success {
                    sstore(add(balances_slot, caller), sub(balance, _am))
                    sstore(0, _am) // dummy operation to mimic Log.AddMessage
                }
            }
        }
        Log.AddMessage(msg.sender, _am, "Collect");
    }
    
    function() public payable {
        Deposit();
    }
}

contract LogFile {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}