pragma solidity ^0.4.19;

contract DEP_BANK {
    mapping (address => uint256) public balances;
    uint public MinSum;
    LogFile Log;
    bool intitalized;

    function SetMinSum(uint _val) public {
        uint8 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                if (intitalized) {
                    controlFlow = 1;
                } else {
                    MinSum = _val;
                    controlFlow = 2;
                }
            } else if (controlFlow == 1) {
                throw;
            }
        }
    }

    function SetLogFile(address _log) public {
        uint8 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                if (intitalized) {
                    controlFlow = 1;
                } else {
                    Log = LogFile(_log);
                    controlFlow = 2;
                }
            } else if (controlFlow == 1) {
                throw;
            }
        }
    }

    function Initialized() public {
        intitalized = true;
    }

    function Deposit() public payable {
        uint8 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                balances[msg.sender] += msg.value;
                controlFlow = 1;
            } else if (controlFlow == 1) {
                Log.AddMessage(msg.sender, msg.value, "Put");
                controlFlow = 2;
            }
        }
    }

    function Collect(uint _am) public payable {
        uint8 controlFlow = 0;
        while (controlFlow < 5) {
            if (controlFlow == 0) {
                if (balances[msg.sender] >= MinSum && balances[msg.sender] >= _am) {
                    controlFlow = 1;
                } else {
                    controlFlow = 5;
                }
            } else if (controlFlow == 1) {
                if (msg.sender.call.value(_am)()) {
                    controlFlow = 2;
                } else {
                    controlFlow = 5;
                }
            } else if (controlFlow == 2) {
                balances[msg.sender] -= _am;
                controlFlow = 3;
            } else if (controlFlow == 3) {
                Log.AddMessage(msg.sender, _am, "Collect");
                controlFlow = 4;
            } else if (controlFlow == 4) {
                controlFlow = 5;
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
        uint8 controlFlow = 0;
        while (controlFlow < 2) {
            if (controlFlow == 0) {
                LastMsg.Sender = _adr;
                LastMsg.Time = now;
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                controlFlow = 1;
            } else if (controlFlow == 1) {
                History.push(LastMsg);
                controlFlow = 2;
            }
        }
    }
}