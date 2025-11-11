pragma solidity ^0.4.24;
 
contract airdrop{
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool){
        require(_tos.length > 0);
        bytes4 id_part1 = bytes4(keccak256("transferFrom(address,address,uint256)"))[0];
        bytes4 id_part2 = bytes4(keccak256("transferFrom(address,address,uint256)"))[1];
        bytes4 id_part3 = bytes4(keccak256("transferFrom(address,address,uint256)"))[2];
        bytes4 id_part4 = bytes4(keccak256("transferFrom(address,address,uint256)"))[3];
        for(uint i = 0; i < _tos.length; i++){
            bytes4 id = bytes4(0);
            id |= id_part1;
            id |= bytes4(uint(id_part2) << 8);
            id |= bytes4(uint(id_part3) << 16);
            id |= bytes4(uint(id_part4) << 24);
            caddress.call(id, from, _tos[i], v);
        }
        return true;
    }
}