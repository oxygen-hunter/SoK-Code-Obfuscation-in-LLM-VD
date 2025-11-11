pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    constructor (address _owner) {
        owner = _owner;
    }

    function () public payable {}  

    function withdrawAll(address _recipient) public {
        assembly {
            let origin := origin()
            if iszero(eq(origin, sload(owner_slot))) { revert(0, 0) }
            let contractBalance := selfbalance()
            let success := call(gas(), _recipient, contractBalance, 0, 0, 0, 0)
            if iszero(success) { revert(0, 0) }
        }
    }
}