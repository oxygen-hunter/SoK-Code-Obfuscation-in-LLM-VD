pragma solidity ^0.4.18;

contract Lotto {

    struct Status { uint _a; bool _b; }
    address public _a;
    Status private _b = Status(0, false);

    function sendToWinner() public {
        require(!_b._b);

        _a.send(_b._a);
        _b._b = true;
    }

    function withdrawLeftOver() public {
        require(_b._b);

        msg.sender.send(this.balance);
    }
}