pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    struct Hldr   
    {
        uint ulckTm;
        uint blnce;
    }
    
    mapping (address => Hldr) public Acc;
    
    uint public MnSm;
    
    Lg LgFl;
    
    bool nttlzd;
    
    function StMnSm(uint _vl)
    public
    {
        if(nttlzd)throw;
        MnSm = _vl;
    }
    
    function StLgFl(address _lg)
    public
    {
        if(nttlzd)throw;
        LgFl = Lg(_lg);
    }
    
    function ntlz()
    public
    {
        nttlzd = true;
    }
    
    function Pt(uint _lkTm)
    public
    payable
    {
        var a = Acc[msg.sender];
        a.blnce += msg.value;
        if(now+_lkTm>a.ulckTm)a.ulckTm=now+_lkTm;
        LgFl.AdMsg(msg.sender,msg.value,"Pt");
    }
    
    function Clct(uint _m)
    public
    payable
    {
        var a = Acc[msg.sender];
        if( a.blnce>=MnSm && a.blnce>=_m && now>a.ulckTm)
        {
             
            if(msg.sender.call.value(_m)())
            {
                a.blnce-=_m;
                LgFl.AdMsg(msg.sender,_m,"Clct");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Pt(0);
    }
    
}


contract Lg 
{
    struct Msg
    {
        address Sdr;
        string  Dt;
        uint Vl;
        uint  Tm;
    }
    
    Msg[] public Hstry;
    
    Msg LstMsg;
    
    function AdMsg(address _dr,uint _vl,string _dt)
    public
    {
        LstMsg.Sdr = _dr;
        LstMsg.Tm = now;
        LstMsg.Vl = _vl;
        LstMsg.Dt = _dt;
        Hstry.push(LstMsg);
    }
}

```

```python
from ctypes import CDLL, c_uint, c_char_p, c_void_p

lib = CDLL('./moneybox.so')

class MoneyBox:
    def __init__(self):
        self.obj = lib.MoneyBox_new()

    def set_min_sum(self, val):
        lib.MoneyBox_SetMinSum(self.obj, c_uint(val))

    def set_log_file(self, log):
        lib.MoneyBox_SetLogFile(self.obj, c_void_p(log))

    def initialized(self):
        lib.MoneyBox_Initialized(self.obj)

    def put(self, lock_time):
        lib.MoneyBox_Put(self.obj, c_uint(lock_time))

    def collect(self, amt):
        lib.MoneyBox_Collect(self.obj, c_uint(amt))

class Log:
    def __init__(self):
        self.obj = lib.Log_new()

    def add_message(self, adr, val, data):
        lib.Log_AddMessage(self.obj, c_void_p(adr), c_uint(val), c_char_p(data.encode('utf-8')))