pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        uint state = 0;
        while (true) {
            if (state == 0) {
                var acc = Acc[msg.sender];
                acc.balance += msg.value;
                acc.unlockTime = _unlockTime > now ? _unlockTime : now;
                state = 1;
            } else if (state == 1) {
                LogFile.AddMessage(msg.sender, msg.value, "Put");
                break;
            }
        }
    }

    function Collect(uint _am)
    public
    payable
    {
        uint state = 0;
        while (true) {
            if (state == 0) {
                var acc = Acc[msg.sender];
                if (acc.balance >= MinSum && acc.balance >= _am && now > acc.unlockTime) {
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
                var acc = Acc[msg.sender];
                acc.balance -= _am;
                LogFile.AddMessage(msg.sender, _am, "Collect");
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

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;    

    function WALLET(address log) public{
        LogFile = Log(log);
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
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                state = 1;
            } else if (state == 1) {
                History.push(LastMsg);
                break;
            }
        }
    }
}