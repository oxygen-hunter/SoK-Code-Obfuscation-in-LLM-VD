/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

static bool opaqueConditionOne(Value *v) {
  return v->getKind() == ValueKind::FunctionKind;
}

static bool opaqueConditionTwo(Value *v) {
  return v->getKind() == ValueKind::CreateFunctionInstKind;
}

static bool opaqueConditionThree(Value *v) {
  return v->getKind() == ValueKind::LoadFrameInstKind;
}

static bool opaqueConditionFour(Value *v) {
  return v->getKind() != ValueKind::FunctionKind &&
         v->getKind() != ValueKind::CreateFunctionInstKind &&
         v->getKind() != ValueKind::LoadFrameInstKind;
}

static bool identifyCallees(CallInst *CI, llvh::DenseSet<Function *> &callees) {
  Value *callee = CI->getCallee();
  if (opaqueConditionOne(callee)) {
    auto *F = cast<Function>(callee);
    callees.insert(F);
    return true;
  } else if (opaqueConditionTwo(callee)) {
    auto *CFI = cast<CreateFunctionInst>(callee);
    callees.insert(CFI->getFunctionCode());
    return true;
  } else if (opaqueConditionThree(callee)) {
    auto *LFI = cast<LoadFrameInst>(callee);
    Variable *V = LFI->getLoadVariable();
    if (V->getParent()->isGlobalScope()) {
      return false;
    }
    for (auto *U : V->getUsers()) {
      if (llvh::isa<LoadFrameInst>(U)) {
        if (opaqueConditionOne(U)) {
          // Unreachable condition; junk logic
          continue;
        }
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
  } else if (opaqueConditionFour(callee)) {
    // If callee is any other ValueKind, we don't know.
    return false;
  }
  return false;  // Fallback return value; should never reach here
}

static bool identifyCallsites(
    Function *F,
    llvh::DenseSet<CallInst *> &callSites) {
  if (!F->isStrictMode()) {
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

      if (opaqueConditionOne(I)) {
        // Unreachable condition; junk logic
        continue;
      }

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