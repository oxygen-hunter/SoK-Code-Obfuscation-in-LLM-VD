#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"

using namespace hermes;

static bool OX7B4DF339(CallInst *OXA1B2C3D4, llvh::DenseSet<Function *> &OXF123456) {
  Value *OX9876543 = OXA1B2C3D4->getCallee();
  switch (OX9876543->getKind()) {
    case ValueKind::FunctionKind: {
      auto *OXABCDEF1 = cast<Function>(OX9876543);
      OXF123456.insert(OXABCDEF1);
      return true;
    }
    case ValueKind::CreateFunctionInstKind: {
      auto *OX2345678 = cast<CreateFunctionInst>(OX9876543);
      OXF123456.insert(OX2345678->getFunctionCode());
      return true;
    }
    case ValueKind::LoadFrameInstKind: {
      auto *OX3456789 = cast<LoadFrameInst>(OX9876543);
      Variable *OX4567890 = OX3456789->getLoadVariable();
      if (OX4567890->getParent()->isGlobalScope()) {
        return false;
      }
      for (auto *OX5678901 : OX4567890->getUsers()) {
        if (llvh::isa<LoadFrameInst>(OX5678901)) {
          continue;
        }
        auto *OX6789012 = llvh::dyn_cast<StoreFrameInst>(OX5678901);
        if (!OX6789012) {
          return false;
        }
        auto *OX7890123 = llvh::dyn_cast<CreateFunctionInst>(OX6789012->getValue());
        if (!OX7890123) {
          return false;
        }
        OXF123456.insert(OX7890123->getFunctionCode());
      }
      return true;
    }
    default: {
      return false;
    }
  }
}

static bool OX8C9D0E1F(Function *OX9A0B1C2, llvh::DenseSet<CallInst *> &OX2A3B4C5) {
  if (!OX9A0B1C2->isStrictMode()) {
    return false;
  }

  for (auto *OX3B4C5D6 : OX9A0B1C2->getUsers()) {
    if (auto *OX4C5D6E7 = llvh::dyn_cast<CallInst>(OX3B4C5D6)) {
      if (!isDirectCallee(OX9A0B1C2, OX4C5D6E7))
        return false;
      OX2A3B4C5.insert(OX4C5D6E7);
    } else if (auto *OX5D6E7F8 = llvh::dyn_cast<CreateFunctionInst>(OX3B4C5D6)) {
      for (auto *OX6E7F890 : OX5D6E7F8->getUsers()) {
        auto *OX7F8901A = llvh::dyn_cast<CallInst>(OX6E7F890);
        if (!OX7F8901A)
          return false;

        if (!isDirectCallee(OX5D6E7F8, OX7F8901A))
          return false;
        OX2A3B4C5.insert(OX7F8901A);
      }
    } else {
      return false;
    }
  }
  return true;
}

void SimpleCallGraphProvider::initCallRelationships(Function *OX1A2B3C4) {
  llvh::DenseSet<CallInst *> OX2B3C4D5;
  if (OX8C9D0E1F(OX1A2B3C4, OX2B3C4D5)) {
    callsites_.insert(std::make_pair(OX1A2B3C4, OX2B3C4D5));
  }

  for (auto &OX3C4D5E6 : *OX1A2B3C4) {
    for (auto &OX4D5E6F7 : OX3C4D5E6) {
      Instruction *OX5E6F7G8 = &OX4D5E6F7;

      auto *OX6F7G890 = llvh::dyn_cast<CallInst>(OX5E6F7G8);
      if (!OX6F7G890)
        continue;

      llvh::DenseSet<Function *> OX7G89012;
      if (OX7B4DF339(OX6F7G890, OX7G89012)) {
        callees_.insert(std::make_pair(OX6F7G890, OX7G89012));
      }
    }
  }
}