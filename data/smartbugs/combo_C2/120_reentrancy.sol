pragma solidity ^0.4.19;

contract PrivateBank {
    mapping(address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    function PrivateBank(address _lib) {
        TransferLog = Log(_lib);
    }

    function Deposit() public payable {
        uint dispatch = 0;
        while (dispatch < 1) {
            if (dispatch == 0) {
                if (msg.value >= MinDeposit) {
                    balances[msg.sender] += msg.value;
                    TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                }
                dispatch = 1;
            }
        }
    }

    function CashOut(uint _am) {
        uint dispatch = 0;
        while (dispatch < 1) {
            if (dispatch == 0) {
                if (_am <= balances[msg.sender]) {
                    dispatch = 1;
                } else {
                    break;
                }
            }
            if (dispatch == 1) {
                if (msg.sender.call.value(_am)()) {
                    balances[msg.sender] -= _am;
                    TransferLog.AddMessage(msg.sender, _am, "CashOut");
                }
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
        uint dispatch = 0;
        while (dispatch < 1) {
            if (dispatch == 0) {
                LastMsg.Sender = _adr;
                LastMsg.Time = now;
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                History.push(LastMsg);
                dispatch = 1;
            }
        }
    }
}