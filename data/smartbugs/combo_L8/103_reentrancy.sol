pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)throw;
        Log = LogFile(_log);
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
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        if(now+_lockTime>acc.unlockTime)acc.unlockTime=now+_lockTime;
        Log.AddMessage(msg.sender,msg.value,"Put");
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
                Log.AddMessage(msg.sender,_am,"Collect");
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

lib = ctypes.CDLL('./path_to_shared_library.so')

class PENNY_BY_PENNY:
    def __init__(self):
        self.Acc = {}
        self.MinSum = 0
        self.Log = LogFile()
        self.intitalized = False

    def SetMinSum(self, _val):
        if self.intitalized:
            raise Exception("Already initialized")
        self.MinSum = _val

    def SetLogFile(self, _log):
        if self.intitalized:
            raise Exception("Already initialized")
        self.Log = _log

    def Initialized(self):
        self.intitalized = True

    def Put(self, _lockTime):
        acc = self.Acc.get("msg_sender", {'balance': 0, 'unlockTime': 0})
        acc['balance'] += lib.msg_value()
        if lib.now() + _lockTime > acc['unlockTime']:
            acc['unlockTime'] = lib.now() + _lockTime
        self.Log.AddMessage("msg_sender", lib.msg_value(), "Put")
        self.Acc["msg_sender"] = acc

    def Collect(self, _am):
        acc = self.Acc.get("msg_sender", {'balance': 0, 'unlockTime': 0})
        if acc['balance'] >= self.MinSum and acc['balance'] >= _am and lib.now() > acc['unlockTime']:
            if lib.msg_sender_call_value(_am):
                acc['balance'] -= _am
                self.Log.AddMessage("msg_sender", _am, "Collect")
                self.Acc["msg_sender"] = acc


class LogFile:
    def __init__(self):
        self.History = []
        self.LastMsg = {'Sender': '', 'Data': '', 'Val': 0, 'Time': 0}

    def AddMessage(self, _adr, _val, _data):
        self.LastMsg['Sender'] = _adr
        self.LastMsg['Time'] = lib.now()
        self.LastMsg['Val'] = _val
        self.LastMsg['Data'] = _data
        self.History.append(self.LastMsg.copy())