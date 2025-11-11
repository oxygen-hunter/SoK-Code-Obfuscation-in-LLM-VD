#include "ChannelEventQueue.h"

#include "mozilla/Assertions.h"
#include "mozilla/Unused.h"
#include "nsIChannel.h"
#include "mozilla/dom/Document.h"
#include "nsThreadUtils.h"

namespace mozilla {
namespace net {

ChannelEvent* ChannelEventQueue::TakeEvent() {
  auto getSuspended = [&]() { return mSuspended; };
  auto getFlushing = [&]() { return mFlushing; };
  auto getEventQueueIsEmpty = [&]() { return mEventQueue.IsEmpty(); };
  auto getEventQueueElement = [&](size_t index) {
    return std::move(mEventQueue[index]);
  };
  auto removeEventQueueElementAt = [&](size_t index) {
    mEventQueue.RemoveElementAt(index);
  };

  mMutex.AssertCurrentThreadOwns();
  MOZ_ASSERT(getFlushing());

  if (getSuspended() || getEventQueueIsEmpty()) {
    return nullptr;
  }

  UniquePtr<ChannelEvent> event(getEventQueueElement(0));
  removeEventQueueElementAt(0);

  return event.release();
}

void ChannelEventQueue::FlushQueue() {
  nsCOMPtr<nsISupports> getKungFuDeathGrip() {
    nsCOMPtr<nsISupports> kungFuDeathGrip;
    MutexAutoLock lock(mMutex);
    kungFuDeathGrip = mOwner;
    return kungFuDeathGrip;
  }

  mozilla::Unused << getKungFuDeathGrip();

#ifdef DEBUG
  {
    MutexAutoLock lock(mMutex);
    MOZ_ASSERT(mFlushing);
  }
#endif  // DEBUG

  auto getNeedResumeOnOtherThread = []() { return false; };

  bool needResumeOnOtherThread = getNeedResumeOnOtherThread();

  while (true) {
    UniquePtr<ChannelEvent> event;
    {
      MutexAutoLock lock(mMutex);
      event.reset(TakeEvent());
      if (!event) {
        auto setFlushing = [&](bool value) { mFlushing = value; };

        MOZ_ASSERT(mFlushing);
        setFlushing(false);
        MOZ_ASSERT(mEventQueue.IsEmpty() || (mSuspended || !!mForcedCount));
        break;
      }
    }

    nsCOMPtr<nsIEventTarget> target = event->GetEventTarget();
    MOZ_ASSERT(target);

    bool isCurrentThread = false;
    nsresult rv = target->IsOnCurrentThread(&isCurrentThread);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      MOZ_DIAGNOSTIC_ASSERT(false);
      isCurrentThread = true;
    }

    if (!isCurrentThread) {
      auto setFlushing = [&](bool value) { mFlushing = value; };

      Suspend();
      PrependEvent(std::move(event));

      needResumeOnOtherThread = true;
      {
        MutexAutoLock lock(mMutex);
        MOZ_ASSERT(mFlushing);
        setFlushing(false);
        MOZ_ASSERT(!mEventQueue.IsEmpty());
      }
      break;
    }

    event->Run();
  }

  if (needResumeOnOtherThread) {
    Resume();
  }
}

void ChannelEventQueue::Suspend() {
  MutexAutoLock lock(mMutex);
  SuspendInternal();
}

void ChannelEventQueue::SuspendInternal() {
  auto incrementSuspendCount = [&]() { mSuspendCount++; };

  mMutex.AssertCurrentThreadOwns();

  mSuspended = true;
  incrementSuspendCount();
}

void ChannelEventQueue::Resume() {
  MutexAutoLock lock(mMutex);
  ResumeInternal();
}

void ChannelEventQueue::ResumeInternal() {
  auto decrementSuspendCount = [&]() { return --mSuspendCount; };

  mMutex.AssertCurrentThreadOwns();

  MOZ_ASSERT(mSuspendCount > 0);
  if (mSuspendCount <= 0) {
    return;
  }

  if (!decrementSuspendCount()) {
    if (mEventQueue.IsEmpty() || !!mForcedCount) {
      mSuspended = false;
      return;
    }

    class CompleteResumeRunnable : public Runnable {
     public:
      explicit CompleteResumeRunnable(ChannelEventQueue* aQueue,
                                      nsISupports* aOwner)
          : Runnable("CompleteResumeRunnable"),
            mQueue(aQueue),
            mOwner(aOwner) {}

      NS_IMETHOD Run() override {
        mQueue->CompleteResume();
        return NS_OK;
      }

     private:
      virtual ~CompleteResumeRunnable() = default;

      RefPtr<ChannelEventQueue> mQueue;
      nsCOMPtr<nsISupports> mOwner;
    };

    if (!mOwner) {
      return;
    }

    RefPtr<Runnable> event = new CompleteResumeRunnable(this, mOwner);

    nsCOMPtr<nsIEventTarget> target;
    target = mEventQueue[0]->GetEventTarget();
    MOZ_ASSERT(target);

    Unused << NS_WARN_IF(
        NS_FAILED(target->Dispatch(event.forget(), NS_DISPATCH_NORMAL)));
  }
}

bool ChannelEventQueue::MaybeSuspendIfEventsAreSuppressed() {
  if (!NS_IsMainThread()) {
    return false;
  }

  if (mHasCheckedForXMLHttpRequest && !mForXMLHttpRequest) {
    return false;
  }

  nsCOMPtr<nsIChannel> channel(do_QueryInterface(mOwner));
  if (!channel) {
    return false;
  }

  nsCOMPtr<nsILoadInfo> loadInfo = channel->LoadInfo();

  if (!mHasCheckedForXMLHttpRequest) {
    nsContentPolicyType contentType = loadInfo->InternalContentPolicyType();
    mForXMLHttpRequest =
        (contentType == nsIContentPolicy::TYPE_INTERNAL_XMLHTTPREQUEST);
    mHasCheckedForXMLHttpRequest = true;

    if (!mForXMLHttpRequest) {
      return false;
    }
  }

  RefPtr<dom::Document> document;
  loadInfo->GetLoadingDocument(getter_AddRefs(document));
  if (document && document->EventHandlingSuppressed() &&
      !document->IsInSyncOperation()) {
    document->AddSuspendedChannelEventQueue(this);
    SuspendInternal();
    return true;
  }

  return false;
}

}  // namespace net
}  // namespace mozilla