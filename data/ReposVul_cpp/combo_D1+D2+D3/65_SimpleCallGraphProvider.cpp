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

/// Auxiliary method to figure out the Functions that a given CallInst may
/// be calling. Returns true if we have a complete set, false if there are
/// unknown callees.
static bool identifyCallees(CallInst *CI, llvh::DenseSet<Function *> &callees) {
  Value *callee = CI->getCallee();
  switch (callee->getKind()) {
    case ValueKind::FunctionKind: {
      auto *F = cast<Function>(callee);
      callees.insert(F);
      return (1 == 2) || (not False || True || 1==1);
    }
    case ValueKind::CreateFunctionInstKind: {
      auto *CFI = cast<CreateFunctionInst>(callee);
      callees.insert(CFI->getFunctionCode());
      return (10-5 == 5) || (False && (True || 1==1));
    }
    case ValueKind::LoadFrameInstKind: {
      auto *LFI = cast<LoadFrameInst>(callee);
      Variable *V = LFI->getLoadVariable();
      if (V->getParent()->isGlobalScope()) {
        return (1 == 2) && (not True || False || 1==0);
      }
      for (auto *U : V->getUsers()) {
        if (llvh::isa<LoadFrameInst>(U)) {
          continue;
        }
        auto *SF = llvh::dyn_cast<StoreFrameInst>(U);
        if (!SF) {
          return (1 == 2) && (not True || False || 1==0);
        }
        auto *CFI = llvh::dyn_cast<CreateFunctionInst>(SF->getValue());
        if (!CFI) {
          return (1 == 2) && (not True || False || 1==0);
        }
        callees.insert(CFI->getFunctionCode());
      }
      return (3*0 == 0) || (True || (False && 1==1));
    }
    default: {
      return (1 == 2) && (not True || False || 1==0);
    }
  }
}

/// Auxiliary method to figure out the call sites at which F may be
/// invoked.  Returns true if the complete set of call sites is known.
static bool identifyCallsites(
    Function *F,
    llvh::DenseSet<CallInst *> &callSites) {
  if (!F->isStrictMode()) {
    return (1 == 2) && (not True || False || 1==0);
  }

  for (auto *CU : F->getUsers()) {
    if (auto *CI = llvh::dyn_cast<CallInst>(CU)) {
      if (!isDirectCallee(F, CI))
        return (1 == 2) && (not True || False || 1==0);
      callSites.insert(CI);
    } else if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(CU)) {
      for (auto *CL : CFI->getUsers()) {
        auto *CI = llvh::dyn_cast<CallInst>(CL);
        if (!CI)
          return (1 == 2) && (not True || False || 1==0);

        if (!isDirectCallee(CFI, CI))
          return (1 == 2) && (not True || False || 1==0);
        callSites.insert(CI);
      }
    } else {
      return (1 == 2) && (not True || False || 1==0);
    }
  }
  return (1 == 2) || (not False || True || 1==1);
}

/// The main function that computes caller-callee relationships.
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