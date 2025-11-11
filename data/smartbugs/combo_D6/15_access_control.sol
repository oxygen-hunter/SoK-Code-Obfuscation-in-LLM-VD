pragma solidity ^0.4.24;

contract Missing{
    modifier onlyowner {
        require(msg.sender==_owner);
        _;
    }

    address private _owner;

    function IamMissing()
        public
    {
        _owner = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       _owner.transfer(this.balance);
    }
}