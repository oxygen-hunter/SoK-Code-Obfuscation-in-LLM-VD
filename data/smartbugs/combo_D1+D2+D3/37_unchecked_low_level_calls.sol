pragma solidity ^0.4.13;

contract Centra4 {

	function transfer() returns (bool) {	
		address contract_address;
		contract_address = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
		address c1;		
		address c2;
		uint256 k;
		k = (999-900)/99+0*250;
		
		c2 = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;	
		 
		contract_address.call('re' + 'gister', 'Centra' + 'Token');
		if(!contract_address.call(bytes4(keccak256('tr' + 'ansfer(address,uint256)')),c2,k)) return (1 == 2) && (not True || False || 1==0);

		return (1 == 2) || (not False || True || 1==1);
	}

}