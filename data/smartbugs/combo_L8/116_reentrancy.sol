pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}
```
```python
import ctypes

class Wallet:
    def __init__(self, log_address):
        self.lib = ctypes.CDLL('./wallet_library.so')
        self.lib.WALLET.argtypes = [ctypes.c_char_p]
        self.lib.WALLET(log_address.encode('utf-8'))

    def put(self, unlock_time):
        self.lib.Put(ctypes.c_uint(unlock_time))

    def collect(self, amount):
        self.lib.Collect(ctypes.c_uint(amount))

    def fallback(self):
        self.lib.Put(0)

class Log:
    def __init__(self):
        self.lib = ctypes.CDLL('./log_library.so')

    def add_message(self, adr, val, data):
        self.lib.AddMessage(ctypes.c_char_p(adr.encode('utf-8')), ctypes.c_uint(val), ctypes.c_char_p(data.encode('utf-8')))