pragma solidity ^0.4.18;

contract AirDropContract {

    function AirDropContract() public {
    }

    modifier validAddress(address addr) {
        require(addr != address(0x0));
        require(addr != address(this));
        _;
    }
    
    function transfer(address c, address[] a, uint[] b)
        public 
        validAddress(c)
        returns (bool){

        require(a.length > 0);
        require(b.length > 0);
        require(a.length == b.length);
        bytes4 d = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint e = 0; e < a.length; e++) {
            c.call(d, msg.sender, a[e], b[e]);
        }
        return true;
    }
}