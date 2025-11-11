pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT {
    mapping (address => uint256) public balances;
    uint public MinSum = 1 ether;
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool intitalized;
    
    function SetMinSum(uint _val) public {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                if (intitalized) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                revert();
            } else if (state == 2) {
                MinSum = _val;
                break;
            }
        }
    }
    
    function SetLogFile(address _log) public {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                if (intitalized) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                revert();
            } else if (state == 2) {
                Log = LogFile(_log);
                break;
            }
        }
    }
    
    function Initialized() public {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                intitalized = true;
                break;
            }
        }
    }
    
    function Deposit() public payable {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                balances[msg.sender] += msg.value;
                state = 1;
            } else if (state == 1) {
                Log.AddMessage(msg.sender, msg.value, "Put");
                break;
            }
        }
    }
    
    function Collect(uint _am) public payable {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                if (balances[msg.sender] >= MinSum && balances[msg.sender] >= _am) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                if (msg.sender.call.value(_am)()) {
                    state = 2;
                } else {
                    break;
                }
            } else if (state == 2) {
                balances[msg.sender] -= _am;
                state = 3;
            } else if (state == 3) {
                Log.AddMessage(msg.sender, _am, "Collect");
                break;
            }
        }
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
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                LastMsg.Sender = _adr;
                state = 1;
            } else if (state == 1) {
                LastMsg.Time = now;
                state = 2;
            } else if (state == 2) {
                LastMsg.Val = _val;
                state = 3;
            } else if (state == 3) {
                LastMsg.Data = _data;
                state = 4;
            } else if (state == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}