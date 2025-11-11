pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) public b;   
    
    uint public M;
    
    LogFile L;
    
    bool i;
    
    function S(uint _v)
    public
    {
        if(i)throw;
        M = _v;
    }
    
    function S(address _l)
    public
    {
        if(i)throw;
        L = LogFile(_l);
    }
    
    function I()
    public
    {
        i = true;
    }
    
    function D()
    public
    payable
    {
        b[msg.sender]+= msg.value;
        L.A(msg.sender,msg.value,"P");
    }
    
    function C(uint _a)
    public
    payable
    {
        if(b[msg.sender]>=M && b[msg.sender]>=_a)
        {
             
            if(msg.sender.call.value(_a)())
            {
                b[msg.sender]-=_a;
                L.A(msg.sender,_a,"C");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        D();
    }
    
}


contract LogFile
{
    struct M
    {
        address S;
        string  D;
        uint V;
        uint  T;
    }
    
    M[] public H;
    
    M L;
    
    function A(address _a,uint _v,string _d)
    public
    {
        L.S = _a;
        L.T = now;
        L.V = _v;
        L.D = _d;
        H.push(L);
    }
}
```

```python
from ctypes import CDLL, c_uint32, c_char_p

lib = CDLL('./c_library.so')

def set_min_sum(value):
    lib.SetMinSum(c_uint32(value))

def set_log_file(log_address):
    lib.SetLogFile(c_char_p(log_address.encode('utf-8')))

def initialized():
    lib.Initialized()

def deposit():
    lib.Deposit()

def collect(amount):
    lib.Collect(c_uint32(amount))