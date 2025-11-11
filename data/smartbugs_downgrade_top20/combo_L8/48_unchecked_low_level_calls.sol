pragma solidity ^0.4.25;
contract demo{
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool){
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i = 0; i < _tos.length; i++){
            caddress.call(id, from, _tos[i], v[i]);
        }
        return true;
    }
}

```

```python
import ctypes

def call_transfer(contract_address, from_address, to_addresses, values):
    # Assuming you have compiled a C library to handle Ethereum interactions
    eth_lib = ctypes.CDLL('./eth_interaction.so')
    eth_lib.transfer.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p), ctypes.POINTER(ctypes.c_uint), ctypes.c_uint]
    
    c_to_addresses = (ctypes.c_char_p * len(to_addresses))(*[bytes(addr, 'utf-8') for addr in to_addresses])
    c_values = (ctypes.c_uint * len(values))(*values)
    
    eth_lib.transfer(bytes(contract_address, 'utf-8'), bytes(from_address, 'utf-8'), c_to_addresses, c_values, len(to_addresses))

# Example usage
# call_transfer("0xContractAddress", "0xFromAddress", ["0xToAddress1", "0xToAddress2"], [100, 200])