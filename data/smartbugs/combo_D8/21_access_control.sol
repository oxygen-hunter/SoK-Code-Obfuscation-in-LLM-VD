pragma solidity ^0.4.24;

contract MyContract {

    address _o0oO0o;

    function MyContract() public {
        _o0oO0o = msg.sender;
    }

    function getOwner() internal view returns (address) {
        return _o0oO0o;
    }

    function sendTo(address o0o0O0, uint oOo0o) public {
        require(tx.origin == getOwner());
        o0o0O0.transfer(oOo0o);
    }

}