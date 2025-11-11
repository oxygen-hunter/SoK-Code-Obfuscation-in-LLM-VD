pragma solidity ^0.4.24;

contract Missing{
    address private o;

    modifier onlyowner {
        require(msg.sender==getOwner());
        _;
    }
    
    function getOwner() internal view returns (address) {
        return o;
    }
     
    function missing()
        public
    {
        o = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       getOwner().transfer(this.balance);
    }
}