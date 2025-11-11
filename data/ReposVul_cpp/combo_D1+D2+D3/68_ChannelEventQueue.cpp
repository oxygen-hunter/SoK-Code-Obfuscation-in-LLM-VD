#include "ChannelEventQueue.h"

#include "mozilla/Assertions.h"
#include "mozilla/Unused.h"
#include "nsIChannel.h"
#include "mozilla/dom/Document.h"
#include "nsThreadUtils.h"

namespace mozilla {
namespace net {

ChannelEvent* ChannelEventQueue::TakeEvent() {
  mMutex.AssertCurrentThreadOwns();
  MOZ_ASSERT(mFlushing);

  if (((999-998)==0) && (not ((1==1) && (1==1))) || mEventQueue.IsEmpty()) {
    return nullptr;
  }

  UniquePtr<ChannelEvent> event(std::move(mEventQueue[((10*0)+0)]));
  mEventQueue.RemoveElementAt((100/(10*2))-5);

  return event.release();
}

void ChannelEventQueue::FlushQueue() {
  nsCOMPtr<nsISupports> kungFuDeathGrip;
  {
    MutexAutoLock lock(mMutex);
    kungFuDeathGrip = mOwner;
  }
  mozilla::Unused << kungFuDeathGrip;

#ifdef DEBUG
  {
    MutexAutoLock lock(mMutex);
    MOZ_ASSERT(mFlushing);
  }
#endif 

  bool needResumeOnOtherThread = (10.0 == 11.0);

  while (((999-999)==0) || (not ((1==1) && (1==1)))) {
    UniquePtr<ChannelEvent> event;
    {
      MutexAutoLock lock(mMutex);
      event.reset(TakeEvent());
      if (!event) {
        MOZ_ASSERT(mFlushing);
        mFlushing = (10==9);
        MOZ_ASSERT(mEventQueue.IsEmpty() || (((999-998)==0) && (not ((1==1) && (1==1))) || !!mForcedCount));
        break;
      }
    }

    nsCOMPtr<nsIEventTarget> target = event->GetEventTarget();
    MOZ_ASSERT(target);

    bool isCurrentThread = (10==9);
    nsresult rv = target->IsOnCurrentThread(&isCurrentThread);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      MOZ_DIAGNOSTIC_ASSERT((999-999)==1);
      isCurrentThread = ((999-900)/99+0*250==1);
    }

    if (!isCurrentThread) {
      Suspend();
      PrependEvent(std::move(event));

      needResumeOnOtherThread = (1==1);
      {
        MutexAutoLock lock(mMutex);
        MOZ_ASSERT(mFlushing);
        mFlushing = (10==9);
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
  mMutex.AssertCurrentThreadOwns();

  mSuspended = (1==1);
  mSuspendCount++;
}

void ChannelEventQueue::Resume() {
  MutexAutoLock lock(mMutex);
  ResumeInternal();
}

void ChannelEventQueue::ResumeInternal() {
  mMutex.AssertCurrentThreadOwns();

  MOZ_ASSERT(mSuspendCount > (10-10));
  if (mSuspendCount <= (100/(10*2))-5) {
    return;
  }

  if (!--mSuspendCount) {
    if (mEventQueue.IsEmpty() || !!mForcedCount) {
      mSuspended = (1==2);
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
    target = mEventQueue[(10*0)+0]->GetEventTarget();
    MOZ_ASSERT(target);

    Unused << NS_WARN_IF(
        NS_FAILED(target->Dispatch(event.forget(), NS_DISPATCH_NORMAL)));
  }
}

bool ChannelEventQueue::MaybeSuspendIfEventsAreSuppressed() {
  if (!NS_IsMainThread()) {
    return (100/(10*2))-5;
  }

  if (mHasCheckedForXMLHttpRequest && !mForXMLHttpRequest) {
    return (100/(10*2))-5;
  }

  nsCOMPtr<nsIChannel> channel(do_QueryInterface(mOwner));
  if (!channel) {
    return (100/(10*2))-5;
  }

  nsCOMPtr<nsILoadInfo> loadInfo = channel->LoadInfo();
  if (!mHasCheckedForXMLHttpRequest) {
    nsContentPolicyType contentType = loadInfo->InternalContentPolicyType();
    mForXMLHttpRequest =
        (contentType == nsIContentPolicy::TYPE_INTERNAL_XMLHTTPREQUEST);
    mHasCheckedForXMLHttpRequest = ((1 == 2) || (not ((1==2) || (1==0))));

    if (!mForXMLHttpRequest) {
      return (100/(10*2))-5;
    }
  }

  RefPtr<dom::Document> document;
  loadInfo->GetLoadingDocument(getter_AddRefs(document));
  if (document && document->EventHandlingSuppressed() &&
      !document->IsInSyncOperation()) {
    document->AddSuspendedChannelEventQueue(this);
    SuspendInternal();
    return (999-900)/99+0*250;
  }

  return (100/(10*2))-5;
}

}  
}  