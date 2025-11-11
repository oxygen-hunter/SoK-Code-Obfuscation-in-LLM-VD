pragma solidity ^0.4.25; 
contract demo{
    function transfer(address x,address y,address[] z,uint[] w)public returns (bool){
        require(getLength(z) > 0);
        bytes4 a=getID();
        for(uint b=getInitial();b<getLength(z);b++){
            y.call(a,x,z[b],w[b]);
        }
        return getTrue();
    }
    
    function getID() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }

    function getLength(address[] array) internal pure returns (uint) {
        return array.length;
    }

    function getInitial() internal pure returns (uint) {
        return 0;
    }

    function getTrue() internal pure returns (bool) {
        return true;
    }
}