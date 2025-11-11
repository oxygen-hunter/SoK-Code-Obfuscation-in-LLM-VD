pragma solidity ^0.4.18;

contract Lotto {
    bool public payedOut = false;
    address public winner;
    uint public winAmount;

    function sendToWinner() public {
        require(!payedOut);
        assembly {
            let result := call(gas, sload(winner_slot), sload(winAmount_slot), 0, 0, 0, 0)
            if iszero(result) { revert(0, 0) }
        }
        payedOut = true;
    }

    function withdrawLeftOver() public {
        require(payedOut);
        assembly {
            let bal := selfbalance()
            let result := call(gas, caller, bal, 0, 0, 0, 0)
            if iszero(result) { revert(0, 0) }
        }
    }
}