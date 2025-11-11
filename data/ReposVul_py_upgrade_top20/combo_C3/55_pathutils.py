import os
import posixpath
from . import log

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.code = []

    def run(self):
        while self.pc < len(self.code):
            instr = self.code[self.pc]
            op = instr[0]
            args = instr[1:]
            if op == "PUSH":
                self.stack.append(args[0])
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == "JMP":
                self.pc = args[0] - 1
            elif op == "JZ":
                if self.stack.pop() == 0:
                    self.pc = args[0] - 1
            elif op == "LOAD":
                self.stack.append(args[0])
            elif op == "STORE":
                self.stack = [args[0]]
            self.pc += 1

def compile_sanitize_path(path):
    vm = VM()
    code = []

    # trailing_slash = "/" if path.endswith("/") else ""
    code.append(("LOAD", path))
    code.append(("LOAD", "/"))
    code.append(("JZ", 7))
    code.append(("PUSH", ""))
    code.append(("JMP", 8))
    code.append(("PUSH", "/"))
    code.append(("STORE", "trailing_slash"))

    # path = posixpath.normpath(path)
    code.append(("LOAD", posixpath.normpath(path)))
    code.append(("STORE", "path"))

    # new_path = "/"
    code.append(("PUSH", "/"))
    code.append(("STORE", "new_path"))

    # for part in path.split("/"):
    code.append(("LOAD", "path"))
    code.append(("PUSH", "/"))
    code.append(("STORE", "split_path"))
    code.append(("LOAD", 0))
    code.append(("STORE", "i"))
    code.append(("LOAD", len(path.split("/"))))
    code.append(("STORE", "len_parts"))
    code.append(("JMP", 28))

    # if not part or part in (".", ".."):
    code.append(("LOAD", "split_path"))
    code.append(("LOAD", "i"))
    code.append(("JZ", 28))
    code.append(("PUSH", "."))
    code.append(("PUSH", ".."))
    code.append(("LOAD", "split_path"))
    code.append(("LOAD", "i"))
    code.append(("SUB", 1))
    code.append(("JZ", 28))
    code.append(("LOAD", "new_path"))
    code.append(("LOAD", "split_path"))
    code.append(("LOAD", "i"))
    code.append(("ADD", 1))
    code.append(("STORE", "new_path"))

    # trailing_slash = "" if new_path.endswith("/") else trailing_slash
    code.append(("LOAD", "new_path"))
    code.append(("LOAD", "/"))
    code.append(("JZ", 40))
    code.append(("LOAD", "trailing_slash"))
    code.append(("JMP", 41))
    code.append(("PUSH", ""))
    code.append(("STORE", "trailing_slash"))

    # return new_path + trailing_slash
    code.append(("LOAD", "new_path"))
    code.append(("LOAD", "trailing_slash"))
    code.append(("ADD", 1))
    code.append(("STORE", "result"))

    vm.code = code
    vm.run()
    return vm.stack[-1]

def compile_is_safe_filesystem_path_component(path):
    vm = VM()
    code = []

    # if not path:
    code.append(("LOAD", path))
    code.append(("JZ", 5))
    code.append(("PUSH", False))
    code.append(("JMP", 6))
    code.append(("PUSH", True))
    code.append(("STORE", "is_safe"))

    # drive, _ = os.path.splitdrive(path)
    code.append(("LOAD", os.path.splitdrive(path)))
    code.append(("STORE", "drive"))

    # if drive:
    code.append(("LOAD", "drive"))
    code.append(("JZ", 13))
    code.append(("PUSH", False))
    code.append(("STORE", "is_safe"))

    # head, _ = os.path.split(path)
    code.append(("LOAD", os.path.split(path)))
    code.append(("STORE", "head"))

    # if head:
    code.append(("LOAD", "head"))
    code.append(("JZ", 20))
    code.append(("PUSH", False))
    code.append(("STORE", "is_safe"))

    # if path in (os.curdir, os.pardir):
    code.append(("LOAD", path))
    code.append(("PUSH", os.curdir))
    code.append(("PUSH", os.pardir))
    code.append(("SUB", 1))
    code.append(("JZ", 28))
    code.append(("PUSH", False))
    code.append(("STORE", "is_safe"))

    # return True
    code.append(("PUSH", True))
    code.append(("STORE", "is_safe"))

    vm.code = code
    vm.run()
    return vm.stack[-1]

def compile_path_to_filesystem(path, base_folder):
    vm = VM()
    code = []

    # sane_path = sanitize_path(path).strip("/")
    code.append(("LOAD", compile_sanitize_path(path).strip("/")))
    code.append(("STORE", "sane_path"))

    # safe_path = base_folder
    code.append(("LOAD", base_folder))
    code.append(("STORE", "safe_path"))

    # if not sane_path:
    code.append(("LOAD", "sane_path"))
    code.append(("JZ", 43))
    code.append(("LOAD", "safe_path"))
    code.append(("STORE", "result"))

    # for part in sane_path.split("/"):
    code.append(("LOAD", "sane_path"))
    code.append(("PUSH", "/"))
    code.append(("STORE", "split_path"))
    code.append(("LOAD", 0))
    code.append(("STORE", "i"))
    code.append(("LOAD", len(sane_path.split("/"))))
    code.append(("STORE", "len_parts"))
    code.append(("JMP", 62))

    # if not is_safe_filesystem_path_component(part):
    code.append(("LOAD", "split_path"))
    code.append(("LOAD", "i"))
    code.append(("JZ", 62))
    code.append(("LOAD", compile_is_safe_filesystem_path_component("split_path")))
    code.append(("JZ", 69))
    code.append(("LOAD", "safe_path"))
    code.append(("LOAD", "split_path"))
    code.append(("LOAD", "i"))
    code.append(("ADD", 1))
    code.append(("STORE", "safe_path"))

    # log.LOGGER.debug("Can't translate path safely to filesystem: %s", path)
    code.append(("LOAD", log.LOGGER.debug("Can't translate path safely to filesystem: %s", path)))
    
    # raise ValueError("Unsafe path")
    code.append(("LOAD", ValueError("Unsafe path")))
    code.append(("STORE", "error"))

    vm.code = code
    vm.run()
    return vm.stack[-1]

# Example usage:
# result = compile_path_to_filesystem('/some/path', '/base/folder')