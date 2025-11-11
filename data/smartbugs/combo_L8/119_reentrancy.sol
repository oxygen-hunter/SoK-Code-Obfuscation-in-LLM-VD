pragma solidity ^0.4.19;

contract PrivateDeposit {
    mapping (address => uint) public b;
    uint public m = 1 ether;
    address public o;
    Log t;
    
    modifier c() {
        require(tx.origin == o);
        _;
    }    
    
    function PrivateDeposit() {
        o = msg.sender;
        t = new Log();
    }
    
    function s(address _l) c {
        t = Log(_l);
    }    
    
    function D()
    public
    payable
    {
        if(msg.value >= m)
        {
            b[msg.sender]+=msg.value;
            t.a(msg.sender,msg.value,"D");
        }
    }
    
    function C(uint _a)
    {
        if(_a<=b[msg.sender])
        {            
            if(msg.sender.call.value(_a)())
            {
                b[msg.sender]-=_a;
                t.a(msg.sender,_a,"C");
            }
        }
    }
    
    function() public payable{}    
}

contract Log {
    struct M
    {
        address s;
        string  d;
        uint v;
        uint  t;
    }
    
    M[] public h;
    M l;
    
    function a(address _a,uint _v,string _d)
    public
    {
        l.s = _a;
        l.t = now;
        l.v = _v;
        l.d = _d;
        h.push(l);
    }
}
```

```c
#include <Python.h>

static PyObject* show_message(PyObject* self, PyObject* args) {
    const char* message;
    if (!PyArg_ParseTuple(args, "s", &message))
        return NULL;
    printf("Log: %s\n", message);
    Py_RETURN_NONE;
}

static PyMethodDef LogMethods[] = {
    {"show_message", show_message, METH_VARARGS, "Displays a log message"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initlog_module(void) {
    (void) Py_InitModule("log_module", LogMethods);
}
```

```python
import ctypes

log_lib = ctypes.CDLL('./log_module.so')

def log_message(msg):
    log_lib.show_message(msg.encode('utf-8'))

log_message("This is a test log message from Python")