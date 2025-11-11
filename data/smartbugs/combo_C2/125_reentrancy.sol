pragma solidity ^0.4.19;

contract BANK_SAFE
{
    mapping (address=>uint256) public balances;   
    uint public MinSum;
    LogFile Log;
    bool intitalized;
    
    function SetMinSum(uint _val) public {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(intitalized)throw;
                MinSum = _val;
                return;
            }
        }
    }
    
    function SetLogFile(address _log) public {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(intitalized)throw;
                Log = LogFile(_log);
                return;
            }
        }
    }
    
    function Initialized() public {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                intitalized = true;
                return;
            }
        }
    }
    
    function Deposit() public payable {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                balances[msg.sender]+= msg.value;
                Log.AddMessage(msg.sender,msg.value,"Put");
                return;
            }
        }
    }
    
    function Collect(uint _am) public payable {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(balances[msg.sender]>=MinSum && balances[msg.sender]>=_am) {
                    dispatcher = 1;
                } else {
                    return;
                }
            } else if (dispatcher == 1) {
                if(msg.sender.call.value(_am)()) {
                    dispatcher = 2;
                } else {
                    return;
                }
            } else if (dispatcher == 2) {
                balances[msg.sender]-=_am;
                Log.AddMessage(msg.sender,_am,"Collect");
                return;
            }
        }
    }
    
    function() public payable {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                Deposit();
                return;
            }
        }
    }  
}

contract LogFile
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data) public {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                LastMsg.Sender = _adr;
                LastMsg.Time = now;
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                History.push(LastMsg);
                return;
            }
        }
    }
}