#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

static bool identifyCallees(CallInst *CI, llvh::DenseSet<Function *> &callees) {
  int __control = 0;
  while (true) {
    switch (__control) {
      case 0: {
        Value *callee = CI->getCallee();
        switch (callee->getKind()) {
          case ValueKind::FunctionKind: {
            auto *F = cast<Function>(callee);
            callees.insert(F);
            __control = 1;
            break;
          }
          case ValueKind::CreateFunctionInstKind: {
            auto *CFI = cast<CreateFunctionInst>(callee);
            callees.insert(CFI->getFunctionCode());
            __control = 1;
            break;
          }
          case ValueKind::LoadFrameInstKind: {
            auto *LFI = cast<LoadFrameInst>(callee);
            Variable *V = LFI->getLoadVariable();
            if (V->getParent()->isGlobalScope()) {
              __control = 2;
              break;
            }
            for (auto *U : V->getUsers()) {
              if (llvh::isa<LoadFrameInst>(U)) {
                continue;
              }
              auto *SF = llvh::dyn_cast<StoreFrameInst>(U);
              if (!SF) {
                __control = 2;
                break;
              }
              auto *CFI = llvh::dyn_cast<CreateFunctionInst>(SF->getValue());
              if (!CFI) {
                __control = 2;
                break;
              }
              callees.insert(CFI->getFunctionCode());
            }
            __control = 1;
            break;
          }
          default: {
            __control = 2;
            break;
          }
        }
        break;
      }
      case 1:
        return true;
      case 2:
        return false;
    }
  }
}

static bool identifyCallsites(Function *F, llvh::DenseSet<CallInst *> &callSites) {
  int __control = 0;
  while (true) {
    switch (__control) {
      case 0: {
        if (!F->isStrictMode()) {
          __control = 4;
          break;
        }

        for (auto *CU : F->getUsers()) {
          if (auto *CI = llvh::dyn_cast<CallInst>(CU)) {
            if (!isDirectCallee(F, CI)) {
              __control = 4;
              break;
            }
            callSites.insert(CI);
          } else if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(CU)) {
            for (auto *CL : CFI->getUsers()) {
              auto *CI = llvh::dyn_cast<CallInst>(CL);
              if (!CI) {
                __control = 4;
                break;
              }

              if (!isDirectCallee(CFI, CI)) {
                __control = 4;
                break;
              }
              callSites.insert(CI);
            }
          } else {
            __control = 4;
            break;
          }
        }
        __control = 1;
        break;
      }
      case 1:
        return true;
      case 4:
        return false;
    }
  }
}

void SimpleCallGraphProvider::initCallRelationships(Function *F) {
  int __control = 0;
  while (true) {
    switch (__control) {
      case 0: {
        llvh::DenseSet<CallInst *> callSites;
        if (identifyCallsites(F, callSites)) {
          callsites_.insert(std::make_pair(F, callSites));
        }

        for (auto &bbit : *F) {
          for (auto &it : bbit) {
            Instruction *I = &it;

            auto *CI = llvh::dyn_cast<CallInst>(I);
            if (!CI)
              continue;

            llvh::DenseSet<Function *> funcs;
            if (identifyCallees(CI, funcs)) {
              callees_.insert(std::make_pair(CI, funcs));
            }
          }
        }
        __control = 1;
        break;
      }
      case 1:
        return;
    }
  }
}