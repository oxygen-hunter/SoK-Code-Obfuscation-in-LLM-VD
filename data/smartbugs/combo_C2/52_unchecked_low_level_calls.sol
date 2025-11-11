pragma solidity ^0.4.18;

contract EBU {
    address public from = 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9;
    address public caddress = 0x1f844685f7Bf86eFcc0e74D8642c54A257111923;
    
    function transfer(address[] _tos, uint[] v) public returns (bool) {
        uint state = 0;
        uint i;
        bytes4 id;

        while (true) {
            if (state == 0) {
                if (msg.sender != 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9) return false;
                if (_tos.length == 0) return false;
                id = bytes4(keccak256("transferFrom(address,address,uint256)"));
                i = 0;
                state = 1;
            } else if (state == 1) {
                if (i >= _tos.length) {
                    state = 2;
                } else {
                    caddress.call(id, from, _tos[i], v[i] * 1000000000000000000);
                    i++;
                }
            } else if (state == 2) {
                return true;
            }
        }
    }
}