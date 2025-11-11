#include "comment.h"
#include "expression.h"
#include "annotation.h"
#include <string>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/regex.hpp>

struct GroupInfo {
	std::string commentString;
	int lineNo;
};

typedef std::vector <GroupInfo> GroupList;

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT
};

class VM {
    std::vector<int> memory;
    std::vector<int> stack;
    int pc;

public:
    VM() : pc(0) {}

    void run(const std::vector<int>& program) {
        while (pc < program.size()) {
            switch (program[pc]) {
                case PUSH: stack.push_back(program[++pc]); break;
                case POP: stack.pop_back(); break;
                case ADD: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP: pc = program[++pc] - 1; break;
                case JZ: {
                    int addr = program[++pc];
                    if (stack.back() == 0) pc = addr - 1;
                    break;
                }
                case LOAD: {
                    int addr = program[++pc];
                    stack.push_back(memory[addr]);
                    break;
                }
                case STORE: {
                    int addr = program[++pc];
                    memory[addr] = stack.back();
                    stack.pop_back();
                    break;
                }
                case CALL: {
                    int addr = program[++pc];
                    stack.push_back(pc + 1);
                    pc = addr - 1;
                    break;
                }
                case HALT: return;
                default: break;
            }
            pc++;
        }
    }
};

void runVMProgram(const std::vector<int>& program) {
    VM vm;
    vm.run(program);
}

static int getLineToStop_VM(const std::string &fulltext) {
    std::vector<int> program = {
        PUSH, 1,
        PUSH, 0,
        LOAD, 0,
        PUSH, 0,
        JZ, 14,
        ADD,
        JMP, 4,
        HALT
    };
    runVMProgram(program);
    return program[1];
}

static std::string getComment_VM(const std::string &fulltext, int line) {
    std::vector<int> program = {
        PUSH, 1,
        PUSH, 0,
        LOAD, 0,
        PUSH, 0,
        JZ, 14,
        ADD,
        JMP, 4,
        HALT
    };
    runVMProgram(program);
    return fulltext.substr(program[1], line);
}

static std::string getDescription_VM(const std::string &fulltext, int line) {
    std::vector<int> program = {
        PUSH, 1,
        PUSH, 0,
        LOAD, 0,
        PUSH, 0,
        JZ, 14,
        ADD,
        JMP, 4,
        HALT
    };
    runVMProgram(program);
    return fulltext.substr(program[1], line);
}

static GroupInfo createGroup_VM(std::string comment, int lineNo) {
    std::vector<int> program = {
        PUSH, 1,
        PUSH, 0,
        LOAD, 0,
        PUSH, 0,
        JZ, 14,
        ADD,
        JMP, 4,
        HALT
    };
    runVMProgram(program);
    GroupInfo groupInfo;
    groupInfo.commentString = comment;
    groupInfo.lineNo = lineNo;
    return groupInfo;
}

static GroupList collectGroups_VM(const std::string &fulltext) {
    std::vector<int> program = {
        PUSH, 1,
        PUSH, 0,
        LOAD, 0,
        PUSH, 0,
        JZ, 14,
        ADD,
        JMP, 4,
        HALT
    };
    runVMProgram(program);
    return GroupList();
}

void CommentParser::collectParameters_VM(const std::string& fulltext, FileModule *root_module) {
    static auto EmptyStringLiteral(std::make_shared<Literal>(Value(std::string(""))));
    GroupList groupList = collectGroups_VM(fulltext);
    int parseTill = getLineToStop_VM(fulltext);
    for (auto &assignment : root_module->scope.assignments) {
        if (!assignment->getExpr()->isLiteral()) continue;
        int firstLine = assignment->location().firstLine();
        if (firstLine >= parseTill || (
            assignment->location().fileName() != "" &&
            assignment->location().fileName() != root_module->getFilename() &&
            assignment->location().fileName() != root_module->getFullpath())) {
            continue;
        }
        AnnotationList *annotationList = new AnnotationList();
        std::string comment = getComment_VM(fulltext, firstLine);
        shared_ptr<Expression> params = CommentParser::parser(comment.c_str());
        if (!params) {
            params = EmptyStringLiteral;
        }
        annotationList->push_back(Annotation("Parameter", params));
        std::string descr = getDescription_VM(fulltext, firstLine - 1);
        if (descr != "") {
            shared_ptr<Expression> expr(new Literal(Value(descr)));
            annotationList->push_back(Annotation("Description", expr));
        }
        for (const auto &groupInfo : boost::adaptors::reverse(groupList)) {
            if (groupInfo.lineNo < firstLine) {
                shared_ptr<Expression> expr(new Literal(Value(groupInfo.commentString)));
                annotationList->push_back(Annotation("Group", expr));
                break;
            }
        }
        assignment->addAnnotations(annotationList);
    }
}