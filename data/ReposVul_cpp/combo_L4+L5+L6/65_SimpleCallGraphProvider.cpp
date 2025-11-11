#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

static bool identifyCallees(CallInst *CI, llvh::DenseSet<Function *> &callees) {
  Value *callee = CI->getCallee();
  if (callee->getKind() == ValueKind::FunctionKind) {
    auto *F = cast<Function>(callee);
    callees.insert(F);
    return true;
  } else if (callee->getKind() == ValueKind::CreateFunctionInstKind) {
    auto *CFI = cast<CreateFunctionInst>(callee);
    callees.insert(CFI->getFunctionCode());
    return true;
  } else if (callee->getKind() == ValueKind::LoadFrameInstKind) {
    auto *LFI = cast<LoadFrameInst>(callee);
    Variable *V = LFI->getLoadVariable();
    if (V->getParent()->isGlobalScope()) {
      return false;
    }
    
    auto processUsers = [&](std::vector<User *> users, size_t index) -> bool {
      if (index >= users.size()) return true;
      auto *U = users[index];
      if (llvh::isa<LoadFrameInst>(U)) {
        return processUsers(users, index + 1);
      }
      auto *SF = llvh::dyn_cast<StoreFrameInst>(U);
      if (!SF) {
        return false;
      }
      auto *CFI = llvh::dyn_cast<CreateFunctionInst>(SF->getValue());
      if (!CFI) {
        return false;
      }
      callees.insert(CFI->getFunctionCode());
      return processUsers(users, index + 1);
    };
    
    if (!processUsers(V->getUsers(), 0)) {
      return false;
    }
    return true;
  } else {
    return false;
  }
}

static bool identifyCallsites(Function *F, llvh::DenseSet<CallInst *> &callSites) {
  if (!F->isStrictMode()) {
    return false;
  }

  auto processUsers = [&](std::vector<User *> users, size_t index) -> bool {
    if (index >= users.size()) return true;
    auto *CU = users[index];
    if (auto *CI = llvh::dyn_cast<CallInst>(CU)) {
      if (!isDirectCallee(F, CI)) return false;
      callSites.insert(CI);
      return processUsers(users, index + 1);
    } else if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(CU)) {
      for (auto *CL : CFI->getUsers()) {
        auto *CI = llvh::dyn_cast<CallInst>(CL);
        if (!CI || !isDirectCallee(CFI, CI)) return false;
        callSites.insert(CI);
      }
      return processUsers(users, index + 1);
    } else {
      return false;
    }
  };
  
  return processUsers(F->getUsers(), 0);
}

void SimpleCallGraphProvider::initCallRelationships(Function *F) {
  llvh::DenseSet<CallInst *> callSites;
  if (identifyCallsites(F, callSites)) {
    callsites_.insert(std::make_pair(F, callSites));
  }

  auto processFunction = [&](Function::iterator bbit, Function::iterator end) -> void {
    if (bbit == end) return;
    auto processInstructions = [&](BasicBlock::iterator it, BasicBlock::iterator end) -> void {
      if (it == end) return;
      Instruction *I = &*it;
      auto *CI = llvh::dyn_cast<CallInst>(I);
      if (CI) {
        llvh::DenseSet<Function *> funcs;
        if (identifyCallees(CI, funcs)) {
          callees_.insert(std::make_pair(CI, funcs));
        }
      }
      processInstructions(std::next(it), end);
    };
    processInstructions(bbit->begin(), bbit->end());
    processFunction(std::next(bbit), end);
  };
  
  processFunction(F->begin(), F->end());
}