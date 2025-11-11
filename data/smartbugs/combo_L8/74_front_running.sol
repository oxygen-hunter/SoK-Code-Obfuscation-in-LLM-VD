pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public hash = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;

    constructor() public payable {}

    function solve(string solution) public {
        require(hash == sha3(solution));
        PythonInterop.transferEther(msg.sender, 1000);
    }
}

library PythonInterop {
    function loadPythonLib() internal returns (address);

    function transferEther(address recipient, uint amount) internal {
        address pythonLib = loadPythonLib();
        bytes4 sig = bytes4(keccak256("transferEther(address,uint256)"));
        require(pythonLib.call(sig, recipient, amount));
    }
}
```
```c
#include <Python.h>

static PyObject *transferEther(PyObject *self, PyObject *args) {
    char *recipient;
    unsigned long amount;
    if (!PyArg_ParseTuple(args, "sk", &recipient, &amount)) {
        return NULL;
    }
    printf("Transferring %lu ether to %s\n", amount, recipient);
    Py_RETURN_NONE;
}

static PyMethodDef Methods[] = {
    {"transferEther", transferEther, METH_VARARGS, "Transfer ether"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initethlib(void) {
    (void) Py_InitModule("ethlib", Methods);
}