pragma solidity ^0.4.19;

contract ETH_FUND {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;
    uint lastBlock;

    function ETH_FUND(address _log) public {
        TransferLog = Log(_log);
    }

    function Deposit() public payable {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.value > MinDeposit) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                balances[msg.sender] += msg.value;
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                lastBlock = block.number;
                state = 2;
            } else if (state == 2) {
                break;
            }
        }
    }

    function CashOut(uint _am) public payable {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (_am <= balances[msg.sender] && block.number > lastBlock) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                if (msg.sender.call.value(_am)()) {
                    state = 2;
                } else {
                    state = 3;
                }
            } else if (state == 2) {
                balances[msg.sender] -= _am;
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
                state = 3;
            } else if (state == 3) {
                break;
            }
        }
    }

    function() public payable {}
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}