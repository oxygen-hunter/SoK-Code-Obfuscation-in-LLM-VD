pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function sendTo(address receiver, uint amount) public {

        require(tx.origin == owner);
        receiver.transfer(amount);
    }
}
```

```python
import ctypes

# Load C DLL
c_lib = ctypes.CDLL('./example.dll')

# Assume C function is void exampleFunction()
c_lib.exampleFunction()

# Continue with Python functionalities
def execute_contract():
    # Python logic here
    pass

execute_contract()