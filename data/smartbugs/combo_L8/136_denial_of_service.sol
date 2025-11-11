pragma solidity 0.4.24;

contract Refunder {
    
address[] private refundAddresses;
mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

     
    function refundAll() public {
        for(uint x; x < refundAddresses.length; x++) {  
         
            require(refundAddresses[x].send(refunds[refundAddresses[x]]));  
        }
    }

    function externalRefund() public {
        string memory command = "python3 refund.py";
        bytes memory commandBytes = bytes(command);
        uint256 len = commandBytes.length;
        bytes memory result = new bytes(len);
        assembly {
            if iszero(call(gas(), 0x0, 0, add(commandBytes, 0x20), len, add(result, 0x20), len)) {
                revert(0, 0)
            }
        }
    }

}
```
```python
# refund.py
import ctypes

def refund():
    libc = ctypes.CDLL("libc.so.6")
    print("Refund process initiated by Python script.")

if __name__ == "__main__":
    refund()