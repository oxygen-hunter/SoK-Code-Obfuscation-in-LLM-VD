pragma solidity ^0.4.15;

contract Unprotected {
    address private o;

    modifier onlyowner {
        require(msg.sender == getOwner());
        _;
    }

    function Unprotected() public {
        setOwner(msg.sender);
    }

    function changeOwner(address _newOwner) public {
        setOwner(_newOwner);
    }

    function getOwner() internal view returns (address) {
        return o;
    }

    function setOwner(address _owner) internal {
        o = _owner;
    }
}