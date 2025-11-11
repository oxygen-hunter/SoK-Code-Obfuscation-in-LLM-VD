#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

static Value *getCallee(CallInst *CI) {
  return CI->getCallee();
}

static Function *castFunction(Value *callee) {
  return cast<Function>(callee);
}

static CreateFunctionInst *castCreateFunctionInst(Value *callee) {
  return cast<CreateFunctionInst>(callee);
}

static LoadFrameInst *castLoadFrameInst(Value *callee) {
  return cast<LoadFrameInst>(callee);
}

static Variable *getLoadVariable(LoadFrameInst *LFI) {
  return LFI->getLoadVariable();
}

static bool identifyCallees(CallInst *CI, llvh::DenseSet<Function *> &callees) {
  Value *callee = getCallee(CI);
  switch (callee->getKind()) {
    case ValueKind::FunctionKind: {
      auto *F = castFunction(callee);
      callees.insert(F);
      return true;
    }
    case ValueKind::CreateFunctionInstKind: {
      auto *CFI = castCreateFunctionInst(callee);
      callees.insert(CFI->getFunctionCode());
      return true;
    }
    case ValueKind::LoadFrameInstKind: {
      auto *LFI = castLoadFrameInst(callee);
      Variable *V = getLoadVariable(LFI);
      if (V->getParent()->isGlobalScope()) {
        return false;
      }
      for (auto *U : V->getUsers()) {
        if (llvh::isa<LoadFrameInst>(U)) {
          continue;
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
      }
      return true;
    }
    default: {
      return false;
    }
  }
}

static bool isStrictMode(Function *F) {
  return F->isStrictMode();
}

static bool identifyCallsites(Function *F, llvh::DenseSet<CallInst *> &callSites) {
  if (!isStrictMode(F)) {
    return false;
  }

  for (auto *CU : F->getUsers()) {
    if (auto *CI = llvh::dyn_cast<CallInst>(CU)) {
      if (!isDirectCallee(F, CI))
        return false;
      callSites.insert(CI);
    } else if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(CU)) {
      for (auto *CL : CFI->getUsers()) {
        auto *CI = llvh::dyn_cast<CallInst>(CL);
        if (!CI)
          return false;

        if (!isDirectCallee(CFI, CI))
          return false;
        callSites.insert(CI);
      }
    } else {
      return false;
    }
  }
  return true;
}

void SimpleCallGraphProvider::initCallRelationships(Function *F) {
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
}