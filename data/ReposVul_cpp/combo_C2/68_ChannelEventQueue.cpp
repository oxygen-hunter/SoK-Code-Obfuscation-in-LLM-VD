/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set sw=2 ts=8 et tw=80 :
 */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ChannelEventQueue.h"

#include "mozilla/Assertions.h"
#include "mozilla/Unused.h"
#include "nsIChannel.h"
#include "mozilla/dom/Document.h"
#include "nsThreadUtils.h"

namespace mozilla {
namespace net {

ChannelEvent* ChannelEventQueue::TakeEvent() {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        mMutex.AssertCurrentThreadOwns();
        MOZ_ASSERT(mFlushing);
        dispatcher = 1;
        break;
      case 1:
        if (mSuspended || mEventQueue.IsEmpty()) {
          return nullptr;
        }
        dispatcher = 2;
        break;
      case 2: {
        UniquePtr<ChannelEvent> event(std::move(mEventQueue[0]));
        mEventQueue.RemoveElementAt(0);
        return event.release();
      }
    }
  }
}

void ChannelEventQueue::FlushQueue() {
  nsCOMPtr<nsISupports> kungFuDeathGrip;
  int dispatcher = 0;
  bool needResumeOnOtherThread = false;

  while (true) {
    switch (dispatcher) {
      case 0: {
        MutexAutoLock lock(mMutex);
        kungFuDeathGrip = mOwner;
        dispatcher = 1;
      } break;
      case 1: {
        mozilla::Unused << kungFuDeathGrip;

#ifdef DEBUG
        {
          MutexAutoLock lock(mMutex);
          MOZ_ASSERT(mFlushing);
        }
#endif  // DEBUG

        dispatcher = 2;
      } break;
      case 2: {
        while (true) {
          UniquePtr<ChannelEvent> event;
          {
            MutexAutoLock lock(mMutex);
            event.reset(TakeEvent());
            if (!event) {
              MOZ_ASSERT(mFlushing);
              mFlushing = false;
              MOZ_ASSERT(mEventQueue.IsEmpty() || (mSuspended || !!mForcedCount));
              dispatcher = 3;
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
            Suspend();
            PrependEvent(std::move(event));

            needResumeOnOtherThread = true;
            {
              MutexAutoLock lock(mMutex);
              MOZ_ASSERT(mFlushing);
              mFlushing = false;
              MOZ_ASSERT(!mEventQueue.IsEmpty());
            }
            dispatcher = 3;
            break;
          }

          event->Run();
        }
      } break;
      case 3: {
        if (needResumeOnOtherThread) {
          Resume();
        }
        return;
      }
    }
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
  if (mSuspendCount <= 0) {
    return;
  }

  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!--mSuspendCount) {
          if (mEventQueue.IsEmpty() || !!mForcedCount) {
            mSuspended = false;
            return;
          }
          dispatcher = 1;
        } else {
          return;
        }
        break;
      case 1: {
        class CompleteResumeRunnable : public Runnable {
         public:
          explicit CompleteResumeRunnable(ChannelEventQueue* aQueue,
                                          nsISupports* aOwner)
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

        if (!mOwner) {
          return;
        }

        RefPtr<Runnable> event = new CompleteResumeRunnable(this, mOwner);

        nsCOMPtr<nsIEventTarget> target;
        target = mEventQueue[0]->GetEventTarget();
        MOZ_ASSERT(target);

        Unused << NS_WARN_IF(
            NS_FAILED(target->Dispatch(event.forget(), NS_DISPATCH_NORMAL)));
        return;
      }
    }
  }
}

bool ChannelEventQueue::MaybeSuspendIfEventsAreSuppressed() {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!NS_IsMainThread()) {
          return false;
        }
        dispatcher = 1;
        break;
      case 1:
        if (mHasCheckedForXMLHttpRequest && !mForXMLHttpRequest) {
          return false;
        }
        dispatcher = 2;
        break;
      case 2: {
        nsCOMPtr<nsIChannel> channel(do_QueryInterface(mOwner));
        if (!channel) {
          return false;
        }
        nsCOMPtr<nsILoadInfo> loadInfo = channel->LoadInfo();
        dispatcher = 3;
      } break;
      case 3:
        if (!mHasCheckedForXMLHttpRequest) {
          nsContentPolicyType contentType = loadInfo->InternalContentPolicyType();
          mForXMLHttpRequest = (contentType == nsIContentPolicy::TYPE_INTERNAL_XMLHTTPREQUEST);
          mHasCheckedForXMLHttpRequest = true;

          if (!mForXMLHttpRequest) {
            return false;
          }
        }
        dispatcher = 4;
        break;
      case 4: {
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
    }
  }
}

}  // namespace net
}  // namespace mozilla