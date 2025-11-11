pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public _=_=0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a; 

    constructor() public payable {}  

    function solve(string solution) public {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, solution)
            let hashSol := keccak256(ptr, mload(solution))
            if eq(sload(0), hashSol) {
                let success := call(gas, caller, 1000 ether, 0, 0, 0, 0)
                switch success
                case 0 { revert(0, 0) }
            }
        }
    }
}