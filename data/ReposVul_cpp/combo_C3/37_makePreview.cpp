#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfPreviewImage.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImathMath.h>
#include <OpenEXR/ImathFun.h>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;

enum Instruction {
    PUSH, POP, ADD, SUB, MUL, DIV, JMP, JZ, LOAD, STORE, HALT
};

struct VirtualMachine {
    vector<int> stack;
    vector<int> memory;
    int pc = 0;
    bool running = true;

    void execute(const vector<int>& program) {
        while (running) {
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
                case MUL: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a * b);
                    break;
                }
                case DIV: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a / b);
                    break;
                }
                case JMP: pc = program[++pc] - 1; break;
                case JZ: {
                    int addr = program[++pc];
                    if (stack.back() == 0) pc = addr - 1;
                    break;
                }
                case LOAD: {
                    int addr = stack.back(); stack.pop_back();
                    stack.push_back(memory[addr]);
                    break;
                }
                case STORE: {
                    int addr = stack.back(); stack.pop_back();
                    memory[addr] = stack.back();
                    break;
                }
                case HALT: running = false; break;
                default: throw runtime_error("Unknown instruction");
            }
            ++pc;
        }
    }
};

float knee(float x, float f) {
    VirtualMachine vm;
    vector<int> program = {
        PUSH, *(int*)&x, PUSH, *(int*)&f, MUL, PUSH, 1, ADD, PUSH, *(int*)&f, DIV, HALT
    };
    vm.execute(program);
    return *(float*)&vm.stack.back();
}

unsigned char gamma(half h, float m) {
    VirtualMachine vm;
    vector<int> program = {
        PUSH, 0, PUSH, *(int*)&h, PUSH, *(int*)&m, MUL, LOAD, ADD, 
        PUSH, *(int*)&(0.184874f), SUB, PUSH, 1, ADD, 
        JMP, 0, HALT
    };
    vm.memory.resize(256);
    vm.execute(program);
    float x = *(float*)&vm.stack.back();
    if (x > 1) x = 1 + knee(x - 1, 0.184874f);
    return (unsigned char)(clamp(pow(x, 0.4545f) * 84.66f, 0.f, 255.f));
}

void generatePreview(const char inFileName[], float exposure, int previewWidth, int& previewHeight, Array2D<PreviewRgba>& previewPixels) {
    RgbaInputFile in(inFileName);
    Box2i dw = in.dataWindow();
    float a = in.pixelAspectRatio();
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;
    Array2D<Rgba> pixels(h, w);
    in.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * w, 1, w);
    in.readPixels(dw.min.y, dw.max.y);
    previewHeight = max(int(h / (w * a) * previewWidth + .5f), 1);
    previewPixels.resizeErase(previewHeight, previewWidth);
    float fx = (previewWidth > 1) ? (float(w - 1) / (previewWidth - 1)) : 1;
    float fy = (previewHeight > 1) ? (float(h - 1) / (previewHeight - 1)) : 1;
    float m = pow(2.f, clamp(exposure + 2.47393f, -20.f, 20.f));
    for (int y = 0; y < previewHeight; ++y) {
        for (int x = 0; x < previewWidth; ++x) {
            PreviewRgba& preview = previewPixels[y][x];
            const Rgba& pixel = pixels[int(y * fy + .5f)][int(x * fx + .5f)];
            preview.r = gamma(pixel.r, m);
            preview.g = gamma(pixel.g, m);
            preview.b = gamma(pixel.b, m);
            preview.a = int(clamp(pixel.a * 255.f, 0.f, 255.f) + .5f);
        }
    }
}

void makePreview(const char inFileName[], const char outFileName[], int previewWidth, float exposure, bool verbose) {
    if (verbose) cout << "generating preview image" << endl;
    Array2D<PreviewRgba> previewPixels;
    int previewHeight;
    generatePreview(inFileName, exposure, previewWidth, previewHeight, previewPixels);
    InputFile in(inFileName);
    Header header = in.header();
    header.setPreviewImage(PreviewImage(previewWidth, previewHeight, &previewPixels[0][0]));
    if (verbose) cout << "copying " << inFileName << " to " << outFileName << endl;
    if (header.hasTileDescription()) {
        TiledOutputFile out(outFileName, header);
        out.copyPixels(in);
    } else {
        OutputFile out(outFileName, header);
        out.copyPixels(in);
    }
    if (verbose) cout << "done." << endl;
}