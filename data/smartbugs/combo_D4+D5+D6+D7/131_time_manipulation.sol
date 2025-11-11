pragma solidity ^0.4.15;

contract EtherLotto {

    struct Constants {
        uint ticket;
        uint fee;
    }

    struct State {
        address bank;
        uint pot;
    }

    Constants private consts = Constants(10, 1);
    State public state;

    function EtherLotto() {
        state.bank = msg.sender;
    }

    function play() payable {
        uint msgValue = msg.value;
        assert(msgValue == consts.ticket);
        
        state.pot += msgValue;

        uint random = uint(sha3(block.timestamp)) % 2;

        if (random == 0) {
            state.bank.transfer(consts.fee);
            msg.sender.transfer(state.pot - consts.fee);
            state.pot = 0;
        }
    }
}