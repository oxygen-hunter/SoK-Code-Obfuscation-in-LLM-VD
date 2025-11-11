import pytest

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
    
    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instruction = self.program[self.pc]
            self.pc += 1
            if instruction[0] == 'PUSH':
                self.stack.append(instruction[1])
            elif instruction[0] == 'POP':
                self.stack.pop()
            elif instruction[0] == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif instruction[0] == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif instruction[0] == 'JMP':
                self.pc = instruction[1]
            elif instruction[0] == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instruction[1]
            elif instruction[0] == 'LOAD':
                self.stack.append(self.stack[instruction[1]])
            elif instruction[0] == 'STORE':
                self.stack[instruction[1]] = self.stack.pop()
            elif instruction[0] == 'RETURN':
                return self.stack.pop()
    
def test_basic_repeater(get_contract_with_gas_estimation):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 0),          # x = 0
        ('PUSH', 6),          # load 6 (loop count)
        ('STORE', 1),         # store loop count
        ('PUSH', 0),          # push loop index
        ('STORE', 2),         # store loop index
        ('LOAD', 2),          # load loop index
        ('LOAD', 1),          # load loop count
        ('SUB',),             # check if loop index < loop count
        ('JZ', 16),           # if not, jump to end
        ('LOAD', 0),          # load x
        ('LOAD', 3),          # load z
        ('ADD',),             # x = x + z
        ('STORE', 0),         # store x
        ('LOAD', 2),          # load loop index
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index
        ('STORE', 2),         # store loop index
        ('JMP', 4),           # jump back to loop start
        ('LOAD', 0),          # load x
        ('RETURN',)           # return x
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 9]  # Initialize stack with x = 0 and z = 9
    result = vm.run()
    assert result == 54

def test_digit_reverser(get_contract_with_gas_estimation):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 0),          # o = 0
        ('STORE', 0),         # store o
        ('PUSH', 0),          # push loop index
        ('STORE', 1),         # store loop index
        ('LOAD', 1),          # load loop index
        ('PUSH', 6),          # load 6 (loop count)
        ('SUB',),             # check if loop index < loop count
        ('JZ', 28),           # if not, jump to end
        ('LOAD', 2),          # load z (which initially is x)
        ('PUSH', 10),         # push 10
        ('MOD',),             # z % 10
        ('STORE', 3),         # store result in dig[i]
        ('LOAD', 2),          # load z
        ('PUSH', 10),         # push 10
        ('DIV',),             # z / 10
        ('STORE', 2),         # store z
        ('LOAD', 1),          # load loop index
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index
        ('STORE', 1),         # store loop index
        ('JMP', 4),           # jump back to loop start
        ('PUSH', 0),          # push loop index
        ('STORE', 1),         # store loop index
        ('LOAD', 1),          # load loop index
        ('PUSH', 6),          # load 6 (loop count)
        ('SUB',),             # check if loop index < loop count
        ('JZ', 56),           # if not, jump to end
        ('LOAD', 0),          # load o
        ('PUSH', 10),         # push 10
        ('MUL',),             # o * 10
        ('LOAD', 3),          # load dig[i]
        ('ADD',),             # o = o * 10 + dig[i]
        ('STORE', 0),         # store o
        ('LOAD', 1),          # load loop index
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index
        ('STORE', 1),         # store loop index
        ('JMP', 32),          # jump back to loop start
        ('LOAD', 0),          # load o
        ('RETURN',)           # return o
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 123456, 123456]  # Initialize stack with o = 0, x = 123456, z = 123456
    result = vm.run()
    assert result == 654321

def test_more_complex_repeater(get_contract_with_gas_estimation):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 0),          # out = 0
        ('STORE', 0),         # store out
        ('PUSH', 0),          # push loop index i
        ('STORE', 1),         # store loop index i
        ('LOAD', 1),          # load loop index i
        ('PUSH', 6),          # load 6 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 36),           # if not, jump to end
        ('LOAD', 0),          # load out
        ('PUSH', 10),         # push 10
        ('MUL',),             # out * 10
        ('STORE', 0),         # store out
        ('PUSH', 0),          # push loop index j
        ('STORE', 2),         # store loop index j
        ('LOAD', 2),          # load loop index j
        ('PUSH', 4),          # load 4 (loop count)
        ('SUB',),             # check if loop index j < loop count
        ('JZ', 32),           # if not, jump to end
        ('LOAD', 0),          # load out
        ('LOAD', 2),          # load j
        ('ADD',),             # out + j
        ('STORE', 0),         # store out
        ('LOAD', 2),          # load loop index j
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index j
        ('STORE', 2),         # store loop index j
        ('JMP', 20),          # jump back to loop start
        ('LOAD', 1),          # load loop index i
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index i
        ('STORE', 1),         # store loop index i
        ('JMP', 4),           # jump back to loop start
        ('LOAD', 0),          # load out
        ('RETURN',)           # return out
    ]
    vm.load_program(bytecode)
    result = vm.run()
    assert result == 666666

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater(get_contract_with_gas_estimation, typ):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 0),          # out = 0
        ('STORE', 0),         # store out
        ('PUSH', 80),         # push start index
        ('STORE', 1),         # store start index
        ('LOAD', 1),          # load loop index
        ('PUSH', 121),        # load end index
        ('SUB',),             # check if index < end index
        ('JZ', 20),           # if not, jump to end
        ('LOAD', 0),          # load out
        ('LOAD', 1),          # load index
        ('ADD',),             # out + index
        ('STORE', 0),         # store out
        ('LOAD', 1),          # load loop index
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index
        ('STORE', 1),         # store loop index
        ('JMP', 6),           # jump back to loop start
        ('LOAD', 0),          # load out
        ('RETURN',)           # return out
    ]
    vm.load_program(bytecode)
    result = vm.run()
    assert result == 4100

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_offset_repeater_2(get_contract_with_gas_estimation, typ):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 0),          # out = 0
        ('STORE', 0),         # store out
        ('LOAD', 1),          # load frm
        ('STORE', 2),         # store start index
        ('LOAD', 2),          # load start index
        ('LOAD', 1),          # load frm
        ('PUSH', 101),        # push 101
        ('ADD',),             # frm + 101
        ('SUB',),             # check if index < frm + 101
        ('JZ', 28),           # if not, jump to end
        ('LOAD', 2),          # load index
        ('LOAD', 3),          # load to
        ('SUB',),             # check if index == to
        ('JZ', 24),           # if yes, break
        ('LOAD', 0),          # load out
        ('LOAD', 2),          # load index
        ('ADD',),             # out + index
        ('STORE', 0),         # store out
        ('LOAD', 2),          # load loop index
        ('PUSH', 1),          # push 1
        ('ADD',),             # increment loop index
        ('STORE', 2),         # store loop index
        ('JMP', 8),           # jump back to loop start
        ('LOAD', 0),          # load out
        ('RETURN',)           # return out
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 100, 99999]  # Initialize stack with frm = 100, to = 99999
    result = vm.run()
    assert result == 15150
    vm.stack = [0, 70, 131]  # Initialize stack with frm = 70, to = 131
    result = vm.run()
    assert result == 6100

def test_loop_call_priv(get_contract_with_gas_estimation):
    vm = SimpleVM()
    bytecode = [
        ('PUSH', 1),          # True
        ('RETURN',)           # return True
    ]
    vm.load_program(bytecode)
    result = vm.run()
    assert result is True

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_repeater(get_contract, typ):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 40),           # if not, jump to end
        ('PUSH', 0),          # push loop index j
        ('STORE', 5),         # store loop index j
        ('LOAD', 5),          # load loop index j
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index j < loop count
        ('JZ', 36),           # if not, jump to outer loop
        ('LOAD', 5),          # load j
        ('PUSH', 5),          # load 5
        ('SUB',),             # check if j > 5
        ('JZ', 32),           # if not, continue
        ('LOAD', 4),          # load i
        ('LOAD', 3),          # load a
        ('SUB',),             # check if i > a
        ('JZ', 28),           # if not, continue
        ('LOAD', 4),          # load i
        ('RETURN',)           # return i
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 6]  # Initialize stack with a = 6
    result = vm.run()
    assert result == 7
    vm.stack = [0, 100]  # Initialize stack with a = 100
    result = vm.run()
    assert result == 31337

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_edge(get_contract, typ):
    vm = SimpleVM()
    # Dummy program to check range
    bytecode = [
        ('PUSH', 1),          # True
        ('RETURN',)           # return True
    ]
    vm.load_program(bytecode)
    result = vm.run()
    assert result is True

@pytest.mark.parametrize("typ", ["uint8", "int128", "uint256"])
def test_for_range_oob_check(get_contract, assert_tx_failed, typ):
    vm = SimpleVM()
    # Dummy program to check out of bound
    bytecode = [
        ('PUSH', 0),          # False
        ('RETURN',)           # return False
    ]
    vm.load_program(bytecode)
    result = vm.run()
    assert result is False

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_return_inside_nested_repeater(get_contract, typ):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 44),           # if not, jump to end
        ('PUSH', 0),          # push loop index x
        ('STORE', 5),         # store loop index x
        ('LOAD', 5),          # load loop index x
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index x < loop count
        ('JZ', 40),           # if not, jump to outer loop
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('LOAD', 3),          # load a
        ('SUB',),             # check if i + x > a
        ('JZ', 36),           # if not, continue
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('RETURN',)           # return i + x
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 14]  # Initialize stack with a = 14
    result = vm.run()
    assert result == 15
    vm.stack = [0, 100]  # Initialize stack with a = 100
    result = vm.run()
    assert result == 31337

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_return_void_nested_repeater(get_contract, typ, val):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 48),           # if not, jump to end
        ('PUSH', 0),          # push loop index x
        ('STORE', 5),         # store loop index x
        ('LOAD', 5),          # load loop index x
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index x < loop count
        ('JZ', 44),           # if not, jump to outer loop
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('LOAD', 3),          # load a
        ('SUB',),             # check if i + x > a
        ('JZ', 40),           # if not, continue
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('STORE', 6),         # store result
        ('RETURN',)           # return
    ]
    vm.load_program(bytecode)
    vm.stack = [0, val]  # Initialize stack with a = val
    result = vm.run()
    if val + 1 >= 19:
        assert result == 31337
    else:
        assert result == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_external_nested_repeater(get_contract, typ, val):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 52),           # if not, jump to end
        ('PUSH', 0),          # push loop index x
        ('STORE', 5),         # store loop index x
        ('LOAD', 5),          # load loop index x
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index x < loop count
        ('JZ', 48),           # if not, jump to outer loop
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('LOAD', 3),          # load a
        ('SUB',),             # check if i + x > a
        ('JZ', 44),           # if not, continue
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('RETURN',)           # return i + x
    ]
    vm.load_program(bytecode)
    vm.stack = [0, val]  # Initialize stack with a = val
    result = vm.run()
    if val + 1 >= 19:
        assert result == 31337
    else:
        assert result == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
@pytest.mark.parametrize("val", range(20))
def test_external_void_nested_repeater(get_contract, typ, val):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 56),           # if not, jump to end
        ('PUSH', 0),          # push loop index x
        ('STORE', 5),         # store loop index x
        ('LOAD', 5),          # load loop index x
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index x < loop count
        ('JZ', 52),           # if not, jump to outer loop
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('LOAD', 3),          # load a
        ('SUB',),             # check if i + x > a
        ('JZ', 48),           # if not, continue
        ('LOAD', 4),          # load i
        ('LOAD', 5),          # load x
        ('ADD',),             # i + x
        ('STORE', 6),         # store result
        ('RETURN',)           # return
    ]
    vm.load_program(bytecode)
    vm.stack = [0, val]  # Initialize stack with a = val
    result = vm.run()
    if val + 1 >= 19:
        assert result == 31337
    else:
        assert result == val + 1

@pytest.mark.parametrize("typ", ["int128", "uint256"])
def test_breaks_and_returns_inside_nested_repeater(get_contract, typ):
    vm = SimpleVM()
    bytecode = [
        ('LOAD', 1),          # load a
        ('STORE', 3),         # store a
        ('PUSH', 0),          # push loop index i
        ('STORE', 4),         # store loop index i
        ('LOAD', 4),          # load loop index i
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index i < loop count
        ('JZ', 60),           # if not, jump to end
        ('PUSH', 0),          # push loop index x
        ('STORE', 5),         # store loop index x
        ('LOAD', 5),          # load loop index x
        ('PUSH', 10),         # load 10 (loop count)
        ('SUB',),             # check if loop index x < loop count
        ('JZ', 56),           # if not, jump to outer loop
        ('LOAD', 3),          # load a
        ('PUSH', 2),          # load 2
        ('SUB',),             # check if a < 2
        ('JZ', 52),           # if not, continue
        ('JMP', 40),          # break
        ('LOAD', 6),          # load result
        ('RETURN',)           # return 6
    ]
    vm.load_program(bytecode)
    vm.stack = [0, 100]  # Initialize stack with a = 100
    result = vm.run()
    assert result == 6
    vm.stack = [0, 1]  # Initialize stack with a = 1
    result = vm.run()
    assert result == 666
    vm.stack = [0, 0]  # Initialize stack with a = 0
    result = vm.run()
    assert result == 31337