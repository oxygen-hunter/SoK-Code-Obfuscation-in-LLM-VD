pragma solidity ^0.4.25;

contract DosOneFunc {

    address[] private dynamicList;

    function getDynamicLimit() private pure returns (uint) {
        return 1500;
    }

    function getPushCount() private pure returns (uint) {
        return 350;
    }

    function ifillArray() public returns (bool){
        if(dynamicList.length < getDynamicLimit()) {
            for(uint i = 0; i < getPushCount(); i++) {
                dynamicList.push(msg.sender);
            }
            return true;
        } else {
            dynamicList = new address[](0);
            return false;
        }
    }
}