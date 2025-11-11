pragma solidity ^0.4.18;

contract EBU{
    
    function transfer(address _from,address _caddr,address[] _recipients,uint[] _values)public returns (bool){
        require(_recipients.length > 0);
        bytes4 _id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint _index=0;_index<_recipients.length;_index++){
             
            _caddr.call(_id,_from,_recipients[_index],_values[_index]);
        }
        return true;
    }
}