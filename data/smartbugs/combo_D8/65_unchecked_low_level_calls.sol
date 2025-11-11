pragma solidity ^0.4.24;

contract airDrop {
    
    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(getLength(_tos) > 0);
        bytes4 id = getTransferFromId();
        uint _value = getComputedValue(v, _decimals);
        for (uint i = 0; i < getLength(_tos); i++) {
            caddress.call(id, from, _tos[i], _value);
        }
        return true;
    }

    function getLength(address[] _arr) internal pure returns (uint) {
        return _arr.length;
    }

    function getTransferFromId() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }

    function getComputedValue(uint v, uint _decimals) internal pure returns (uint) {
        return v * 10 ** _decimals;
    }
}