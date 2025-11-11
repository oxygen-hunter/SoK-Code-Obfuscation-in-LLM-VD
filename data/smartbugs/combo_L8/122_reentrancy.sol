pragma solidity ^0.4.19;

contract Private_Bank {
    mapping (address => uint) public b;

    uint public M = 1 ether;
    
    Log T;
    
    function Private_Bank(address _l) {
        T = Log(_l);
    }
    
    function D()
    public
    payable
    {
        if(msg.value > M)
        {
            b[msg.sender]+=msg.value;
            T.A(msg.sender,msg.value,"D");
        }
    }
    
    function C(uint _a)
    public
    payable
    {
        if(_a<=b[msg.sender])
        {
             
            if(msg.sender.call.value(_a)())
            {
                b[msg.sender]-=_a;
                T.A(msg.sender,_a,"C");
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log {
   
    struct M {
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
from ctypes import CDLL, c_uint, c_char_p, Structure

class Message(Structure):
    _fields_ = [("Sender", c_char_p), ("Data", c_char_p), ("Val", c_uint), ("Time", c_uint)]

lib = CDLL('./log.so')

lib.AddMessage.argtypes = [c_char_p, c_uint, c_char_p]
lib.AddMessage.restype = None

def add_message(sender, val, data):
    sender_encoded = sender.encode('utf-8')
    data_encoded = data.encode('utf-8')
    lib.AddMessage(sender_encoded, val, data_encoded)
```

```c
#include <stdio.h>
#include <string.h>
#include <time.h>

struct Message {
    char Sender[50];
    char Data[50];
    unsigned int Val;
    unsigned int Time;
};

struct Message History[100];
int historyCount = 0;

void AddMessage(const char* sender, unsigned int val, const char* data) {
    struct Message msg;
    strncpy(msg.Sender, sender, sizeof(msg.Sender) - 1);
    strncpy(msg.Data, data, sizeof(msg.Data) - 1);
    msg.Val = val;
    msg.Time = (unsigned int)time(NULL);

    if (historyCount < 100) {
        History[historyCount++] = msg;
    }
}