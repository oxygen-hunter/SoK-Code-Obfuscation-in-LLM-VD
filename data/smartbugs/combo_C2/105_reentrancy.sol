pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    Log LogFile;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if(intitalized) {
                    state = 1;
                    continue;
                } else {
                    state = 2;
                    continue;
                }
            }
            if (state == 1) {
                throw;
                break;
            }
            if (state == 2) {
                MinSum = _val;
                break;
            }
        }
    }
    
    function SetLogFile(address _log)
    public
    {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if(intitalized) {
                    state = 1;
                    continue;
                } else {
                    state = 2;
                    continue;
                }
            }
            if (state == 1) {
                throw;
                break;
            }
            if (state == 2) {
                LogFile = Log(_log);
                break;
            }
        }
    }
    
    function Initialized()
    public
    {
        intitalized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        uint state = 0;
        var acc = Acc[msg.sender];
        while (true) {
            if (state == 0) {
                acc.balance += msg.value;
                state = 1;
                continue;
            }
            if (state == 1) {
                if(now+_lockTime>acc.unlockTime) {
                    acc.unlockTime=now+_lockTime;
                }
                state = 2;
                continue;
            }
            if (state == 2) {
                LogFile.AddMessage(msg.sender,msg.value,"Put");
                break;
            }
        }
    }
    
    function Collect(uint _am)
    public
    payable
    {
        uint state = 0;
        var acc = Acc[msg.sender];
        while (true) {
            if (state == 0) {
                if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime) {
                    state = 1;
                    continue;
                } else {
                    break;
                }
            }
            if (state == 1) {
                if(msg.sender.call.value(_am)()) {
                    state = 2;
                    continue;
                } else {
                    break;
                }
            }
            if (state == 2) {
                acc.balance-=_am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
                break;
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Put(0);
    }
    
}


contract Log 
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
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        uint state = 0;
        while (true) {
            if (state == 0) {
                LastMsg.Sender = _adr;
                LastMsg.Time = now;
                state = 1;
                continue;
            }
            if (state == 1) {
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                state = 2;
                continue;
            }
            if (state == 2) {
                History.push(LastMsg);
                break;
            }
        }
    }
}