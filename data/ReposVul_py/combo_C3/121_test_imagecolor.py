import pytest
from PIL import Image, ImageColor

# Define VM instruction set
PUSH = 0
POP = 1
CALL = 2
CMP = 3
JMP = 4
JZ = 5

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0

    def execute(self, instructions):
        while self.pc < len(instructions):
            inst = instructions[self.pc]
            if inst[0] == PUSH:
                self.stack.append(inst[1])
            elif inst[0] == POP:
                self.stack.pop()
            elif inst[0] == CALL:
                func = inst[1]
                func(*self.stack[-len(inst[2]):])
                self.stack = self.stack[:-len(inst[2])]
            elif inst[0] == CMP:
                a, b = self.stack.pop(), self.stack.pop()
                self.stack.append(a == b)
            elif inst[0] == JMP:
                self.pc = inst[1]
                continue
            elif inst[0] == JZ:
                if not self.stack.pop():
                    self.pc = inst[1]
                    continue
            self.pc += 1

def vm_assert(condition):
    assert condition

def vm_raise(exception):
    raise exception

def test_hash():
    instr = [
        (PUSH, (255, 0, 0)), (PUSH, "#f00"), (CALL, ImageColor.getrgb, ["#f00"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 0)), (PUSH, "#0f0"), (CALL, ImageColor.getrgb, ["#0f0"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 255)), (PUSH, "#00f"), (CALL, ImageColor.getrgb, ["#00f"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0, 0)), (PUSH, "#f000"), (CALL, ImageColor.getrgb, ["#f000"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 0, 0)), (PUSH, "#0f00"), (CALL, ImageColor.getrgb, ["#0f00"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 255, 0)), (PUSH, "#00f0"), (CALL, ImageColor.getrgb, ["#00f0"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 0, 255)), (PUSH, "#000f"), (CALL, ImageColor.getrgb, ["#000f"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (222, 0, 0)), (PUSH, "#de0000"), (CALL, ImageColor.getrgb, ["#de0000"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 222, 0)), (PUSH, "#00de00"), (CALL, ImageColor.getrgb, ["#00de00"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 222)), (PUSH, "#0000de"), (CALL, ImageColor.getrgb, ["#0000de"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (222, 0, 0, 0)), (PUSH, "#de000000"), (CALL, ImageColor.getrgb, ["#de000000"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 222, 0, 0)), (PUSH, "#00de0000"), (CALL, ImageColor.getrgb, ["#00de0000"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 222, 0)), (PUSH, "#0000de00"), (CALL, ImageColor.getrgb, ["#0000de00"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 0, 222)), (PUSH, "#000000de"), (CALL, ImageColor.getrgb, ["#000000de"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "#DEF"), (PUSH, "#def"), (CALL, ImageColor.getrgb, ["#def"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "#CDEF"), (PUSH, "#cdef"), (CALL, ImageColor.getrgb, ["#cdef"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "#DEFDEF"), (PUSH, "#defdef"), (CALL, ImageColor.getrgb, ["#defdef"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "#CDEFCDEF"), (PUSH, "#cdefcdef"), (CALL, ImageColor.getrgb, ["#cdefcdef"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "#fo0"), (CALL, ImageColor.getrgb, ["#fo0"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#fo00"), (CALL, ImageColor.getrgb, ["#fo00"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#fo0000"), (CALL, ImageColor.getrgb, ["#fo0000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#fo000000"), (CALL, ImageColor.getrgb, ["#fo000000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#f0000"), (CALL, ImageColor.getrgb, ["#f0000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#f000000"), (CALL, ImageColor.getrgb, ["#f000000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#f00000000"), (CALL, ImageColor.getrgb, ["#f00000000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#f000000000"), (CALL, ImageColor.getrgb, ["#f000000000"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "#f00000 "), (CALL, ImageColor.getrgb, ["#f00000 "]), (CALL, vm_raise, [ValueError]),
    ]
    vm = VM()
    vm.execute(instr)

def test_colormap():
    instr = [
        (PUSH, (0, 0, 0)), (PUSH, "black"), (CALL, ImageColor.getrgb, ["black"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 255, 255)), (PUSH, "white"), (CALL, ImageColor.getrgb, ["white"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 255, 255)), (PUSH, "WHITE"), (CALL, ImageColor.getrgb, ["WHITE"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "black "), (CALL, ImageColor.getrgb, ["black "]), (CALL, vm_raise, [ValueError]),
    ]
    vm = VM()
    vm.execute(instr)

def test_functions():
    instr = [
        (PUSH, (255, 0, 0)), (PUSH, "rgb(255,0,0)"), (CALL, ImageColor.getrgb, ["rgb(255,0,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 0)), (PUSH, "rgb(0,255,0)"), (CALL, ImageColor.getrgb, ["rgb(0,255,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 255)), (PUSH, "rgb(0,0,255)"), (CALL, ImageColor.getrgb, ["rgb(0,0,255)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "rgb(100%,0%,0%)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%,0%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 0)), (PUSH, "rgb(0%,100%,0%)"), (CALL, ImageColor.getrgb, ["rgb(0%,100%,0%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 255)), (PUSH, "rgb(0%,0%,100%)"), (CALL, ImageColor.getrgb, ["rgb(0%,0%,100%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0, 0)), (PUSH, "rgba(255,0,0,0)"), (CALL, ImageColor.getrgb, ["rgba(255,0,0,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 0, 0)), (PUSH, "rgba(0,255,0,0)"), (CALL, ImageColor.getrgb, ["rgba(0,255,0,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 255, 0)), (PUSH, "rgba(0,0,255,0)"), (CALL, ImageColor.getrgb, ["rgba(0,0,255,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 0, 255)), (PUSH, "rgba(0,0,0,255)"), (CALL, ImageColor.getrgb, ["rgba(0,0,0,255)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsl(0,100%,50%)"), (CALL, ImageColor.getrgb, ["hsl(0,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsl(360,100%,50%)"), (CALL, ImageColor.getrgb, ["hsl(360,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 255)), (PUSH, "hsl(180,100%,50%)"), (CALL, ImageColor.getrgb, ["hsl(180,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsv(0,100%,100%)"), (CALL, ImageColor.getrgb, ["hsv(0,100%,100%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsv(360,100%,100%)"), (CALL, ImageColor.getrgb, ["hsv(360,100%,100%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 255)), (PUSH, "hsv(180,100%,100%)"), (CALL, ImageColor.getrgb, ["hsv(180,100%,100%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "hsb(0,100%,50%)"), (PUSH, "hsv(0,100%,50%)"), (CALL, ImageColor.getrgb, ["hsv(0,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (254, 3, 3)), (PUSH, "hsl(0.1,99.2%,50.3%)"), (CALL, ImageColor.getrgb, ["hsl(0.1,99.2%,50.3%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsl(360.,100.0%,50%)"), (CALL, ImageColor.getrgb, ["hsl(360.,100.0%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (253, 2, 2)), (PUSH, "hsv(0.1,99.2%,99.3%)"), (CALL, ImageColor.getrgb, ["hsv(0.1,99.2%,99.3%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsv(360.,100.0%,100%)"), (CALL, ImageColor.getrgb, ["hsv(360.,100.0%,100%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "RGB(255,0,0)"), (PUSH, "rgb(255,0,0)"), (CALL, ImageColor.getrgb, ["rgb(255,0,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "RGB(100%,0%,0%)"), (PUSH, "rgb(100%,0%,0%)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%,0%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "RGBA(255,0,0,0)"), (PUSH, "rgba(255,0,0,0)"), (CALL, ImageColor.getrgb, ["rgba(255,0,0,0)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "HSL(0,100%,50%)"), (PUSH, "hsl(0,100%,50%)"), (CALL, ImageColor.getrgb, ["hsl(0,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "HSV(0,100%,50%)"), (PUSH, "hsv(0,100%,50%)"), (CALL, ImageColor.getrgb, ["hsv(0,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "HSB(0,100%,50%)"), (PUSH, "hsb(0,100%,50%)"), (CALL, ImageColor.getrgb, ["hsb(0,100%,50%)"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "rgb(  255  ,  0  ,  0  )"), (CALL, ImageColor.getrgb, ["rgb(  255  ,  0  ,  0  )"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "rgb(  100%  ,  0%  ,  0%  )"), (CALL, ImageColor.getrgb, ["rgb(  100%  ,  0%  ,  0%  )"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0, 0)), (PUSH, "rgba(  255  ,  0  ,  0  ,  0  )"), (CALL, ImageColor.getrgb, ["rgba(  255  ,  0  ,  0  ,  0  )"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsl(  0  ,  100%  ,  50%  )"), (CALL, ImageColor.getrgb, ["hsl(  0  ,  100%  ,  50%  )"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 0, 0)), (PUSH, "hsv(  0  ,  100%  ,  100%  )"), (CALL, ImageColor.getrgb, ["hsv(  0  ,  100%  ,  100%  )"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, "rgb(255,0)"), (CALL, ImageColor.getrgb, ["rgb(255,0)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgb(255,0,0,0)"), (CALL, ImageColor.getrgb, ["rgb(255,0,0,0)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgb(100%,0%)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgb(100%,0%,0)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%,0)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgb(100%,0%,0 %)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%,0 %)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgb(100%,0%,0%,0%)"), (CALL, ImageColor.getrgb, ["rgb(100%,0%,0%,0%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgba(255,0,0)"), (CALL, ImageColor.getrgb, ["rgba(255,0,0)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "rgba(255,0,0,0,0)"), (CALL, ImageColor.getrgb, ["rgba(255,0,0,0,0)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsl(0,100%)"), (CALL, ImageColor.getrgb, ["hsl(0,100%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsl(0,100%,0%,0%)"), (CALL, ImageColor.getrgb, ["hsl(0,100%,0%,0%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsl(0%,100%,50%)"), (CALL, ImageColor.getrgb, ["hsl(0%,100%,50%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsl(0,100,50%)"), (CALL, ImageColor.getrgb, ["hsl(0,100,50%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsl(0,100%,50)"), (CALL, ImageColor.getrgb, ["hsl(0,100%,50)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsv(0,100%)"), (CALL, ImageColor.getrgb, ["hsv(0,100%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsv(0,100%,0%,0%)"), (CALL, ImageColor.getrgb, ["hsv(0,100%,0%,0%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsv(0%,100%,50%)"), (CALL, ImageColor.getrgb, ["hsv(0%,100%,50%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsv(0,100,50%)"), (CALL, ImageColor.getrgb, ["hsv(0,100,50%)"]), (CALL, vm_raise, [ValueError]),
        (PUSH, "hsv(0,100%,50)"), (CALL, ImageColor.getrgb, ["hsv(0,100%,50)"]), (CALL, vm_raise, [ValueError]),
    ]
    vm = VM()
    vm.execute(instr)

def test_rounding_errors():
    instr = [
        (PUSH, ImageColor.colormap), (CALL, lambda cmap: [(Image.new("RGB", (1, 1), color).convert("L").getpixel((0, 0)), ImageColor.getcolor(color, "L")) for color in cmap], [None]), 
        (CALL, lambda results: all(vm_assert(expected == actual) for expected, actual in results), [True]),
        (PUSH, (0, 255, 115)), (PUSH, "rgba(0, 255, 115, 33)"), (CALL, ImageColor.getcolor, ["rgba(0, 255, 115, 33)", "RGB"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 0, 0, 255)), (PUSH, "black"), (CALL, ImageColor.getcolor, ["black", "RGBA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 255, 255, 255)), (PUSH, "white"), (CALL, ImageColor.getcolor, ["white", "RGBA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255, 115, 33)), (PUSH, "rgba(0, 255, 115, 33)"), (CALL, ImageColor.getcolor, ["rgba(0, 255, 115, 33)", "RGBA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 0), (PUSH, "black"), (CALL, ImageColor.getcolor, ["black", "L"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 255), (PUSH, "white"), (CALL, ImageColor.getcolor, ["white", "L"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 163), (PUSH, "rgba(0, 255, 115, 33)"), (CALL, ImageColor.getcolor, ["rgba(0, 255, 115, 33)", "L"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 0), (PUSH, "black"), (CALL, ImageColor.getcolor, ["black", "1"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 255), (PUSH, "white"), (CALL, ImageColor.getcolor, ["white", "1"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, 163), (PUSH, "rgba(0, 255, 115, 33)"), (CALL, ImageColor.getcolor, ["rgba(0, 255, 115, 33)", "1"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (0, 255)), (PUSH, "black"), (CALL, ImageColor.getcolor, ["black", "LA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (255, 255)), (PUSH, "white"), (CALL, ImageColor.getcolor, ["white", "LA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
        (PUSH, (163, 33)), (PUSH, "rgba(0, 255, 115, 33)"), (CALL, ImageColor.getcolor, ["rgba(0, 255, 115, 33)", "LA"]), (CMP, None), (CALL, vm_assert, [True]), (POP, None),
    ]
    vm = VM()
    vm.execute(instr)

def test_color_too_long():
    instr = [
        (PUSH, "hsl(" + "1" * 100 + ")"), (CALL, ImageColor.getrgb, ["hsl(" + "1" * 100 + ")"]), (CALL, vm_raise, [ValueError]),
    ]
    vm = VM()
    vm.execute(instr)