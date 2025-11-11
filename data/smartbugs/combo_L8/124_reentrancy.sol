pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    function PrivateBank(address _log) {
        TransferLog = Log(_log);
    }

    function Deposit() public payable {
        if(msg.value >= MinDeposit) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }

    function CashOut(uint _am) {
        if(_am <= balances[msg.sender]) {
            if(msg.sender.call.value(_am)()) {
                balances[msg.sender] -= _am;
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
            }
        }
    }

    function() public payable {}
}

contract Log 
{
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }
    
    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}

```

```c
#include <Python.h>

static PyObject* py_bank_deposit(PyObject* self, PyObject* args) {
    // Dummy C function to simulate a callback
    return Py_BuildValue("s", "Deposit processed in C");
}

static PyObject* py_bank_cashout(PyObject* self, PyObject* args) {
    // Dummy C function to simulate a callback
    return Py_BuildValue("s", "Cashout processed in C");
}

static PyMethodDef BankMethods[] = {
    {"deposit", py_bank_deposit, METH_VARARGS, "Process deposit"},
    {"cashout", py_bank_cashout, METH_VARARGS, "Process cashout"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initbank(void) {
    (void) Py_InitModule("bank", BankMethods);
}
```

```python
import ctypes

bank = ctypes.CDLL("./bank.so")
deposit_result = bank.deposit()
cashout_result = bank.cashout()

print(ctypes.c_char_p(deposit_result).value.decode("utf-8"))
print(ctypes.c_char_p(cashout_result).value.decode("utf-8"))