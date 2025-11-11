#include "ChannelEventQueue.h"
#include "mozilla/Assertions.h"
#include "mozilla/Unused.h"
#include "nsIChannel.h"
#include "mozilla/dom/Document.h"
#include "nsThreadUtils.h"

namespace mozilla {
namespace net {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
public:
  VM() : pc(0), sp(-1), halted(false) {}

  void execute(const std::vector<int>& bytecode) {
    while (!halted) {
      switch (bytecode[pc]) {
        case PUSH:
          stack[++sp] = bytecode[++pc];
          break;
        case POP:
          --sp;
          break;
        case ADD:
          stack[sp - 1] += stack[sp];
          --sp;
          break;
        case SUB:
          stack[sp - 1] -= stack[sp];
          --sp;
          break;
        case JMP:
          pc = bytecode[++pc] - 1;
          break;
        case JZ:
          if (stack[sp--] == 0) {
            pc = bytecode[++pc] - 1;
          } else {
            ++pc;
          }
          break;
        case LOAD:
          stack[++sp] = memory[bytecode[++pc]];
          break;
        case STORE:
          memory[bytecode[++pc]] = stack[sp--];
          break;
        case CALL:
          callStack.push(pc + 1);
          pc = bytecode[++pc] - 1;
          break;
        case RET:
          pc = callStack.top();
          callStack.pop();
          break;
        case HALT:
          halted = true;
          break;
        default:
          break;
      }
      ++pc;
    }
  }

  int stack[256];
  int memory[256];
  int pc;
  int sp;
  bool halted;
  std::stack<int> callStack;
};

ChannelEvent* ChannelEventQueue::TakeEvent() {
  std::vector<int> bytecode = {
    PUSH, 1, LOAD, 0, JZ, 9, LOAD, 1, STORE, 2, HALT
  };
  VM vm;
  vm.memory[0] = mSuspended || mEventQueue.IsEmpty();
  vm.memory[1] = (int)std::move(mEventQueue[0].get());
  vm.execute(bytecode);

  if (vm.memory[2]) return nullptr;

  UniquePtr<ChannelEvent> event(std::move(mEventQueue[0]));
  mEventQueue.RemoveElementAt(0);

  return event.release();
}

void ChannelEventQueue::FlushQueue() {
  nsCOMPtr<nsISupports> kungFuDeathGrip;
  {
    MutexAutoLock lock(mMutex);
    kungFuDeathGrip = mOwner;
  }
  mozilla::Unused << kungFuDeathGrip;

  std::vector<int> bytecode = {
    PUSH, 1, LOAD, 0, JZ, 20, LOAD, 1, JZ, 20, CALL, 10, HALT,
    PUSH, 0, LOAD, 2, JZ, 17, LOAD, 3, STORE, 4, POP, JMP, 5, HALT
  };

  VM vm;
  vm.memory[0] = mFlushing;
  vm.memory[2] = true;

  while (true) {
    {
      MutexAutoLock lock(mMutex);
      vm.memory[1] = (int)TakeEvent();
    }
    vm.execute(bytecode);

    if (!vm.memory[4]) break;

    nsCOMPtr<nsIEventTarget> target = ((ChannelEvent*)vm.memory[1])->GetEventTarget();
    MOZ_ASSERT(target);

    bool isCurrentThread = false;
    nsresult rv = target->IsOnCurrentThread(&isCurrentThread);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      MOZ_DIAGNOSTIC_ASSERT(false);
      isCurrentThread = true;
    }

    if (!isCurrentThread) {
      Suspend();
      PrependEvent(UniquePtr<ChannelEvent>((ChannelEvent*)vm.memory[1]));
      {
        MutexAutoLock lock(mMutex);
        mFlushing = false;
      }
      Resume();
      break;
    }

    ((ChannelEvent*)vm.memory[1])->Run();
  }
}

void ChannelEventQueue::Suspend() {
  MutexAutoLock lock(mMutex);
  SuspendInternal();
}

void ChannelEventQueue::SuspendInternal() {
  mMutex.AssertCurrentThreadOwns();
  mSuspended = true;
  mSuspendCount++;
}

void ChannelEventQueue::Resume() {
  MutexAutoLock lock(mMutex);
  ResumeInternal();
}

void ChannelEventQueue::ResumeInternal() {
  mMutex.AssertCurrentThreadOwns();
  MOZ_ASSERT(mSuspendCount > 0);
  if (mSuspendCount <= 0) return;

  if (!--mSuspendCount) {
    if (mEventQueue.IsEmpty() || !!mForcedCount) {
      mSuspended = false;
      return;
    }

    class CompleteResumeRunnable : public Runnable {
     public:
      explicit CompleteResumeRunnable(ChannelEventQueue* aQueue, nsISupports* aOwner)
          : Runnable("CompleteResumeRunnable"), mQueue(aQueue), mOwner(aOwner) {}

      NS_IMETHOD Run() override {
        mQueue->CompleteResume();
        return NS_OK;
      }

     private:
      virtual ~CompleteResumeRunnable() = default;
      RefPtr<ChannelEventQueue> mQueue;
      nsCOMPtr<nsISupports> mOwner;
    };

    if (!mOwner) return;

    RefPtr<Runnable> event = new CompleteResumeRunnable(this, mOwner);
    nsCOMPtr<nsIEventTarget> target = mEventQueue[0]->GetEventTarget();
    MOZ_ASSERT(target);
    Unused << NS_WARN_IF(NS_FAILED(target->Dispatch(event.forget(), NS_DISPATCH_NORMAL)));
  }
}

bool ChannelEventQueue::MaybeSuspendIfEventsAreSuppressed() {
  if (!NS_IsMainThread()) return false;

  if (mHasCheckedForXMLHttpRequest && !mForXMLHttpRequest) return false;

  nsCOMPtr<nsIChannel> channel(do_QueryInterface(mOwner));
  if (!channel) return false;

  nsCOMPtr<nsILoadInfo> loadInfo = channel->LoadInfo();
  if (!mHasCheckedForXMLHttpRequest) {
    nsContentPolicyType contentType = loadInfo->InternalContentPolicyType();
    mForXMLHttpRequest = (contentType == nsIContentPolicy::TYPE_INTERNAL_XMLHTTPREQUEST);
    mHasCheckedForXMLHttpRequest = true;

    if (!mForXMLHttpRequest) return false;
  }

  RefPtr<dom::Document> document;
  loadInfo->GetLoadingDocument(getter_AddRefs(document));
  if (document && document->EventHandlingSuppressed() && !document->IsInSyncOperation()) {
    document->AddSuspendedChannelEventQueue(this);
    SuspendInternal();
    return true;
  }

  return false;
}

}  // namespace net
}  // namespace mozilla