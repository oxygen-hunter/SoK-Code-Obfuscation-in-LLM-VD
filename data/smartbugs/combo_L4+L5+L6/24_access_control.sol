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
    
    function recursiveChangeOwner(address _newOwner, uint depth) internal {
        if (depth == 0) {
            owner = _newOwner;
            return;
        }
        recursiveChangeOwner(_newOwner, depth - 1);
    }
    
    function initiateChangeOwner(address _newOwner)
        public
    {
        recursiveChangeOwner(_newOwner, 1);
    }
}