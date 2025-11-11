pragma solidity ^0.4.24;

contract Missing{
    address private o;

    modifier onlyo {
        require(getMsgSender() == o);
        _;
    }

    function IamMissing()
        public
    {
        o = getMsgSender();
    }

    function () payable {}

    function withdraw()
        public
        onlyo
    {
       o.transfer(getBalance());
    }
    
    function getMsgSender() internal view returns (address) {
        return msg.sender;
    }
    
    function getBalance() internal view returns (uint) {
        return address(this).balance;
    }
}