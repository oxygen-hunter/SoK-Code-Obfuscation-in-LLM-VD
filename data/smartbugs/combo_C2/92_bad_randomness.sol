pragma solidity ^0.4.16;

contract Ethraffle_v4b {
    struct Contestant {
        address addr;
        uint raffleId;
    }

    event RaffleResult(
        uint raffleId,
        uint winningNumber,
        address winningAddress,
        address seed1,
        address seed2,
        uint seed3,
        bytes32 randHash
    );

    event TicketPurchase(
        uint raffleId,
        address contestant,
        uint number
    );

    event TicketRefund(
        uint raffleId,
        address contestant,
        uint number
    );

    uint public constant prize = 2.5 ether;
    uint public constant fee = 0.03 ether;
    uint public constant totalTickets = 50;
    uint public constant pricePerTicket = (prize + fee) / totalTickets;  
    address feeAddress;

    bool public paused = false;
    uint public raffleId = 1;
    uint public blockNumber = block.number;
    uint nextTicket = 0;
    mapping (uint => Contestant) contestants;
    uint[] gaps;

    function Ethraffle_v4b() public {
        feeAddress = msg.sender;
    }

    function () payable public {
        buyTickets();
    }

    function buyTickets() payable public {
        uint state = 0;
        uint moneySent;
        uint currTicket;
        while (true) {
            if (state == 0) {
                if (paused) {
                    state = 1;
                    continue;
                }
                moneySent = msg.value;
                state = 2;
            } else if (state == 1) {
                msg.sender.transfer(msg.value);
                return;
            } else if (state == 2) {
                state = 3;
            } else if (state == 3) {
                if (moneySent >= pricePerTicket && nextTicket < totalTickets) {
                    state = 4;
                    continue;
                }
                state = 7;
            } else if (state == 4) {
                if (gaps.length > 0) {
                    currTicket = gaps[gaps.length-1];
                    gaps.length--;
                    state = 5;
                    continue;
                }
                currTicket = nextTicket++;
                state = 5;
            } else if (state == 5) {
                contestants[currTicket] = Contestant(msg.sender, raffleId);
                TicketPurchase(raffleId, msg.sender, currTicket);
                moneySent -= pricePerTicket;
                state = 3;
            } else if (state == 7) {
                if (nextTicket == totalTickets) {
                    state = 8;
                    continue;
                }
                state = 10;
            } else if (state == 8) {
                chooseWinner();
                state = 10;
            } else if (state == 10) {
                if (moneySent > 0) {
                    msg.sender.transfer(moneySent);
                }
                return;
            }
        }
    }

    function chooseWinner() private {
        uint state = 0;
        address seed1;
        address seed2;
        uint seed3;
        bytes32 randHash;
        uint winningNumber;
        address winningAddress;
        while (true) {
            if (state == 0) {
                seed1 = contestants[uint(block.coinbase) % totalTickets].addr;
                state = 1;
            } else if (state == 1) {
                seed2 = contestants[uint(msg.sender) % totalTickets].addr;
                state = 2;
            } else if (state == 2) {
                seed3 = block.difficulty;
                randHash = keccak256(seed1, seed2, seed3);
                winningNumber = uint(randHash) % totalTickets;
                winningAddress = contestants[winningNumber].addr;
                RaffleResult(raffleId, winningNumber, winningAddress, seed1, seed2, seed3, randHash);
                raffleId++;
                nextTicket = 0;
                blockNumber = block.number;
                winningAddress.transfer(prize);
                feeAddress.transfer(fee);
                return;
            }
        }
    }

    function getRefund() public {
        uint state = 0;
        uint refund = 0;
        uint i = 0;
        while (true) {
            if (state == 0) {
                if (i < totalTickets) {
                    state = 1;
                    continue;
                }
                state = 3;
            } else if (state == 1) {
                if (msg.sender == contestants[i].addr && raffleId == contestants[i].raffleId) {
                    refund += pricePerTicket;
                    contestants[i] = Contestant(address(0), 0);
                    gaps.push(i);
                    TicketRefund(raffleId, msg.sender, i);
                }
                i++;
                state = 0;
            } else if (state == 3) {
                if (refund > 0) {
                    msg.sender.transfer(refund);
                }
                return;
            }
        }
    }

    function endRaffle() public {
        uint state = 0;
        uint i = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == feeAddress) {
                    state = 1;
                    continue;
                }
                return;
            } else if (state == 1) {
                paused = true;
                state = 2;
            } else if (state == 2) {
                if (i < totalTickets) {
                    state = 3;
                    continue;
                }
                state = 5;
            } else if (state == 3) {
                if (raffleId == contestants[i].raffleId) {
                    TicketRefund(raffleId, contestants[i].addr, i);
                    contestants[i].addr.transfer(pricePerTicket);
                }
                i++;
                state = 2;
            } else if (state == 5) {
                RaffleResult(raffleId, totalTickets, address(0), address(0), address(0), 0, 0);
                raffleId++;
                nextTicket = 0;
                blockNumber = block.number;
                gaps.length = 0;
                return;
            }
        }
    }

    function togglePause() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == feeAddress) {
                    paused = !paused;
                }
                return;
            }
        }
    }

    function kill() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == feeAddress) {
                    selfdestruct(feeAddress);
                }
                return;
            }
        }
    }
}