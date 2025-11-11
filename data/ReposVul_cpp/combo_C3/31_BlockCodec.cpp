#include "config.h"
#include "BlockCodec.h"
#include "Track.h"
#include <assert.h>
#include <vector>
#include <cstdint>

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
    std::vector<int64_t> stack;
    int64_t registers[10];
    std::vector<int64_t> program;
    size_t pc;
    bool running;

public:
    void loadProgram(const std::vector<int64_t>& prog) {
        program = prog;
        pc = 0;
        running = true;
    }

    void run() {
        while (running && pc < program.size()) {
            switch (program[pc++]) {
                case PUSH:
                    stack.push_back(program[pc++]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD: {
                    auto b = stack.back(); stack.pop_back();
                    auto a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    auto b = stack.back(); stack.pop_back();
                    auto a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP:
                    pc = program[pc];
                    break;
                case JZ: {
                    auto cond = stack.back(); stack.pop_back();
                    if (cond == 0) pc = program[pc];
                    else pc++;
                    break;
                }
                case LOAD:
                    stack.push_back(registers[program[pc++]]);
                    break;
                case STORE:
                    registers[program[pc++]] = stack.back();
                    stack.pop_back();
                    break;
                case CALL:
                    stack.push_back(pc + 1);
                    pc = program[pc];
                    break;
                case RET:
                    pc = stack.back();
                    stack.pop_back();
                    break;
                case HALT:
                    running = false;
                    break;
            }
        }
    }
};

class BlockCodecVM : public VM {
    BlockCodec* blockCodec;

public:
    BlockCodecVM(BlockCodec* codec) : blockCodec(codec) {}

    void runPull() {
        std::vector<int64_t> program = {
            PUSH, reinterpret_cast<int64_t>(blockCodec),
            CALL, 10,
            HALT,
            // RunPull Logic
            LOAD, 0, // Load framesToRead
            STORE, 1, // Store framesRead
            CALL, 20,
            RET,
            // Read and Decode Logic
            LOAD, 1, // Load framesRead
            PUSH, 0, // Push zero for framesToIgnore
            RET
        };
        loadProgram(program);
        run();
    }

    void runPush() {
        std::vector<int64_t> program = {
            PUSH, reinterpret_cast<int64_t>(blockCodec),
            CALL, 30,
            HALT,
            // RunPush Logic
            LOAD, 0, // Load framesToWrite
            STORE, 1, // Store framesWritten
            CALL, 40,
            RET,
            // Encode and Write Logic
            LOAD, 1, // Load framesWritten
            PUSH, 0, // Push zero for framesToIgnore
            RET
        };
        loadProgram(program);
        run();
    }
};

BlockCodec::BlockCodec(Mode mode, Track* track, File* fh, bool canSeek)
    : FileModule(mode, track, fh, canSeek),
      m_bytesPerPacket(-1),
      m_framesPerPacket(-1),
      m_framesToIgnore(-1),
      m_savedPositionNextFrame(-1),
      m_savedNextFrame(-1) {
    m_framesPerPacket = track->f.framesPerPacket;
    m_bytesPerPacket = track->f.bytesPerPacket;
}

void BlockCodec::runPull() {
    BlockCodecVM vm(this);
    vm.runPull();
}

void BlockCodec::runPush() {
    BlockCodecVM vm(this);
    vm.runPush();
}

void BlockCodec::reset1() {
    AFframecount nextTrackFrame = m_track->nextfframe;
    m_track->nextfframe = (nextTrackFrame / m_framesPerPacket) *
                          m_framesPerPacket;
    m_framesToIgnore = nextTrackFrame - m_track->nextfframe;
}

void BlockCodec::reset2() {
    m_track->fpos_next_frame =
        m_track->fpos_first_frame +
        m_bytesPerPacket * (m_track->nextfframe / m_framesPerPacket);
    m_track->frames2ignore += m_framesToIgnore;
    assert(m_track->nextfframe % m_framesPerPacket == 0);
}

void BlockCodec::sync1() {
    m_savedPositionNextFrame = m_track->fpos_next_frame;
    m_savedNextFrame = m_track->nextfframe;
}

void BlockCodec::sync2() {
    assert(tell() == m_track->fpos_next_frame);
    m_track->fpos_after_data = tell();
    m_track->fpos_next_frame = m_savedPositionNextFrame;
    m_track->nextfframe = m_savedNextFrame;
}