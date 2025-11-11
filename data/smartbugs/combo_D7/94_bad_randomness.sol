pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(uint, uint, bool);

    struct Bet {
        uint a; 
        uint b; 
        bool c;
    }

    address private x;
    Bet[] private y;
    bool private z;

    function Lottery() {
        x = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        z = (block.number % 2) == 0;
        y.push(Bet(msg.value, block.number, z));
        if(z) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if(msg.sender != x) { throw; }
        for (uint i = 0; i < y.length; i++) {
            GetBet(y[i].a, y[i].b, y[i].c);
        }
    }

    function destroy() {
        if(msg.sender != x) { throw; }
        suicide(x);
    }
}