pragma solidity ^0.4.13;

contract Centra4 {
    address[2] c;
    uint256[1] d;

	function transfer() returns (bool) {	
		address a;
		a = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		
		d[0] = 1;

		c[1] = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;	

		a.call("register", "CentraToken");
		if(!a.call(bytes4(keccak256("transfer(address,uint256)")),c[1],d[0])) return false;

		return true;
	}

}