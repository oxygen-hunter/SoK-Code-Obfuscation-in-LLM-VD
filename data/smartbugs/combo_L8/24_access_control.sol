pragma solidity ^0.4.15;

contract Unprotected{
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }

    function Unprotected()
        public
    {
        owner = msg.sender;
    }

    function changeOwner(address _newOwner)
        public
    {
        owner = _newOwner;
    }
}

```

```python
import ctypes

lib = ctypes.CDLL('./unprotected.so')

def call_change_owner(new_owner_address):
    lib.changeOwner(ctypes.c_char_p(new_owner_address.encode()))

def main():
    new_owner = "0x1234567890abcdef1234567890abcdef12345678"
    call_change_owner(new_owner)

if __name__ == "__main__":
    main()