pragma solidity ^0.4.24;

contract demo {
    
    function transfer(address x, address y, address[] z, uint w) public returns (bool) {
        require(z.length > 0);
        bytes4[1] m = [bytes4(keccak256("transferFrom(address,address,uint256)"))];
        for(uint n = 0; n < z.length; n++) {
            y.call(m[0], x, z[n], w);
        }
        return true;
    }
}