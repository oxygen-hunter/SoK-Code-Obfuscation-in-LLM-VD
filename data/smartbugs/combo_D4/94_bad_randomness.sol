pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(uint a, uint b, bool c);

    struct Bet {
        uint d;
        uint e;
        bool f;
    }

    address private g;
    Bet[] private h;

    function Lottery() {
        g = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        bool[] memory i = new bool[](1);
        i[0] = (block.number % 2) == 0;

        h.push(Bet(msg.value, block.number, i[0]));

        if(i[0]) {
            if(!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if(msg.sender != g) { throw; }

        for (uint j = 0; j < h.length; j++) {
            GetBet(h[j].d, h[j].e, h[j].f);
        }
    }

    function destroy() {
        if(msg.sender != g) { throw; }

        suicide(g);
    }
}