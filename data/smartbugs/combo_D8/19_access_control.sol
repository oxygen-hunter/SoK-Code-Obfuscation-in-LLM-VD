pragma solidity ^0.4.24;

contract Missing{
    address private dynamicOwner;

    modifier onlyowner {
        require(getSender() == getOwner());
        _;
    }
    
    function getSender() internal view returns (address) {
        return msg.sender;
    }

    function getOwner() internal view returns (address) {
        return dynamicOwner;
    }
    
    function setOwner(address _owner) internal {
        dynamicOwner = _owner;
    }
     
    function Constructor()
        public
    {
        setOwner(getSender());
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       getOwner().transfer(address(this).balance);
    }

}