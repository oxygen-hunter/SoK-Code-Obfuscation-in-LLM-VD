/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/DOMParser.h"

#include "nsNetUtil.h"
#include "nsDOMString.h"
#include "MainThreadUtils.h"
#include "SystemPrincipal.h"
#include "nsIScriptGlobalObject.h"
#include "nsIStreamListener.h"
#include "nsStringStream.h"
#include "nsCRT.h"
#include "nsStreamUtils.h"
#include "nsContentUtils.h"
#include "nsDOMJSUtils.h"
#include "nsError.h"
#include "nsPIDOMWindow.h"
#include "mozilla/BasePrincipal.h"
#include "mozilla/LoadInfo.h"
#include "mozilla/NullPrincipal.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/Document.h"
#include "mozilla/dom/ScriptSettings.h"

using namespace mozilla;
using namespace mozilla::dom;

DOMParser::DOMParser(nsIGlobalObject* aOwner, nsIPrincipal* aDocPrincipal,
                     nsIURI* aDocumentURI, nsIURI* aBaseURI)
    : mOwner(aOwner),
      mPrincipal(aDocPrincipal),
      mDocumentURI(aDocumentURI),
      mBaseURI(aBaseURI),
      mForceEnableXULXBL(false),
      mForceEnableDTD(false) {
  MOZ_ASSERT(aDocPrincipal);
  MOZ_ASSERT(aDocumentURI);
}

DOMParser::~DOMParser() = default;

// QueryInterface implementation for DOMParser
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(DOMParser)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(DOMParser, mOwner)

NS_IMPL_CYCLE_COLLECTING_ADDREF(DOMParser)
NS_IMPL_CYCLE_COLLECTING_RELEASE(DOMParser)

already_AddRefed<Document> DOMParser::ParseFromString(const nsAString& aStr,
                                                      SupportedType aType,
                                                      ErrorResult& aRv) {
  int dispatchIndex = 0;
  while (true) {
    switch (dispatchIndex) {
      case 0: {
        if (aType == SupportedType::Text_html) {
          dispatchIndex = 1;
          break;
        }
        dispatchIndex = 6;
        break;
      }
      case 1: {
        nsCOMPtr<Document> document = SetUpDocument(DocumentFlavorHTML, aRv);
        if (NS_WARN_IF(aRv.Failed())) {
          return nullptr;
        }
        dispatchIndex = 2;
        break;
      }
      case 2: {
        if (mForceEnableXULXBL) {
          document->ForceEnableXULXBL();
        }
        dispatchIndex = 3;
        break;
      }
      case 3: {
        if (mForceEnableDTD) {
          document->ForceSkipDTDSecurityChecks();
        }
        dispatchIndex = 4;
        break;
      }
      case 4: {
        nsresult rv = nsContentUtils::ParseDocumentHTML(aStr, document, false);
        if (NS_WARN_IF(NS_FAILED(rv))) {
          aRv.Throw(rv);
          return nullptr;
        }
        dispatchIndex = 5;
        break;
      }
      case 5: {
        return document.forget();
      }
      case 6: {
        nsAutoCString utf8str;
        if (!AppendUTF16toUTF8(aStr, utf8str, mozilla::fallible)) {
          aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
          return nullptr;
        }
        dispatchIndex = 7;
        break;
      }
      case 7: {
        nsCOMPtr<nsIInputStream> stream;
        nsresult rv = NS_NewByteInputStream(getter_AddRefs(stream), utf8str,
                                            NS_ASSIGNMENT_DEPEND);
        if (NS_WARN_IF(NS_FAILED(rv))) {
          aRv.Throw(rv);
          return nullptr;
        }
        dispatchIndex = 8;
        break;
      }
      case 8: {
        return ParseFromStream(stream, u"UTF-8"_ns, utf8str.Length(), aType, aRv);
      }
    }
  }
}

already_AddRefed<Document> DOMParser::ParseFromSafeString(const nsAString& aStr,
                                                          SupportedType aType,
                                                          ErrorResult& aRv) {
  nsCOMPtr<nsIPrincipal> docPrincipal = mPrincipal;
  if (mOwner && mOwner->PrincipalOrNull()) {
    mPrincipal = mOwner->PrincipalOrNull();
  }
  RefPtr<Document> ret = ParseFromString(aStr, aType, aRv);
  mPrincipal = docPrincipal;
  return ret.forget();
}

already_AddRefed<Document> DOMParser::ParseFromBuffer(const Uint8Array& aBuf,
                                                      SupportedType aType,
                                                      ErrorResult& aRv) {
  aBuf.ComputeState();
  return ParseFromBuffer(Span(aBuf.Data(), aBuf.Length()), aType, aRv);
}

already_AddRefed<Document> DOMParser::ParseFromBuffer(Span<const uint8_t> aBuf,
                                                      SupportedType aType,
                                                      ErrorResult& aRv) {
  nsCOMPtr<nsIInputStream> stream;
  nsresult rv = NS_NewByteInputStream(
      getter_AddRefs(stream),
      Span(reinterpret_cast<const char*>(aBuf.Elements()), aBuf.Length()),
      NS_ASSIGNMENT_DEPEND);
  if (NS_FAILED(rv)) {
    aRv.Throw(rv);
    return nullptr;
  }
  return ParseFromStream(stream, VoidString(), aBuf.Length(), aType, aRv);
}

already_AddRefed<Document> DOMParser::ParseFromStream(nsIInputStream* aStream,
                                                      const nsAString& aCharset,
                                                      int32_t aContentLength,
                                                      SupportedType aType,
                                                      ErrorResult& aRv) {
  bool svg = (aType == SupportedType::Image_svg_xml);
  if (aType != SupportedType::Text_xml &&
      aType != SupportedType::Application_xml &&
      aType != SupportedType::Application_xhtml_xml && !svg) {
    aRv.Throw(NS_ERROR_NOT_IMPLEMENTED);
    return nullptr;
  }
  int dispatcher = 0;
  nsCOMPtr<nsIInputStream> stream = aStream;
  while (true) {
    switch (dispatcher) {
      case 0: {
        if (!NS_InputStreamIsBuffered(stream)) {
          dispatcher = 1;
          break;
        }
        dispatcher = 3;
        break;
      }
      case 1: {
        nsCOMPtr<nsIInputStream> bufferedStream;
        nsresult rv = NS_NewBufferedInputStream(getter_AddRefs(bufferedStream),
                                                stream.forget(), 4096);
        if (NS_WARN_IF(NS_FAILED(rv))) {
          aRv.Throw(rv);
          return nullptr;
        }
        stream = bufferedStream;
        dispatcher = 3;
        break;
      }
      case 3: {
        nsCOMPtr<Document> document =
            SetUpDocument(svg ? DocumentFlavorSVG : DocumentFlavorLegacyGuess, aRv);
        if (NS_WARN_IF(aRv.Failed())) {
          return nullptr;
        }
        dispatcher = 4;
        break;
      }
      case 4: {
        nsCOMPtr<nsIChannel> parserChannel;
        NS_NewInputStreamChannel(
            getter_AddRefs(parserChannel), mDocumentURI,
            nullptr,  // aStream
            mPrincipal, nsILoadInfo::SEC_FORCE_INHERIT_PRINCIPAL,
            nsIContentPolicy::TYPE_OTHER,
            nsDependentCSubstring(SupportedTypeValues::GetString(aType)));
        if (NS_WARN_IF(!parserChannel)) {
          aRv.Throw(NS_ERROR_UNEXPECTED);
          return nullptr;
        }
        dispatcher = 5;
        break;
      }
      case 5: {
        if (!DOMStringIsNull(aCharset)) {
          parserChannel->SetContentCharset(NS_ConvertUTF16toUTF8(aCharset));
        }
        dispatcher = 6;
        break;
      }
      case 6: {
        nsCOMPtr<nsIStreamListener> listener;
        if (mForceEnableXULXBL) {
          document->ForceEnableXULXBL();
        }
        dispatcher = 7;
        break;
      }
      case 7: {
        if (mForceEnableDTD) {
          document->ForceSkipDTDSecurityChecks();
        }
        dispatcher = 8;
        break;
      }
      case 8: {
        nsresult rv =
            document->StartDocumentLoad(kLoadAsData, parserChannel, nullptr, nullptr,
                                        getter_AddRefs(listener), false);
        if (NS_FAILED(rv) || !listener) {
          aRv.Throw(NS_ERROR_FAILURE);
          return nullptr;
        }
        dispatcher = 9;
        break;
      }
      case 9: {
        nsresult status;
        nsresult rv = listener->OnStartRequest(parserChannel);
        if (NS_FAILED(rv)) parserChannel->Cancel(rv);
        parserChannel->GetStatus(&status);
        if (NS_SUCCEEDED(rv) && NS_SUCCEEDED(status)) {
          rv = listener->OnDataAvailable(parserChannel, stream, 0, aContentLength);
          if (NS_FAILED(rv)) parserChannel->Cancel(rv);
          parserChannel->GetStatus(&status);
        }
        dispatcher = 10;
        break;
      }
      case 10: {
        nsresult rv = listener->OnStopRequest(parserChannel, status);
        if (NS_FAILED(rv)) {
          aRv.Throw(NS_ERROR_FAILURE);
          return nullptr;
        }
        return document.forget();
      }
    }
  }
}

/*static */
already_AddRefed<DOMParser> DOMParser::Constructor(const GlobalObject& aOwner,
                                                   ErrorResult& rv) {
  MOZ_ASSERT(NS_IsMainThread());
  int flowControl = 0;
  nsCOMPtr<nsIPrincipal> docPrincipal;
  nsCOMPtr<nsIURI> documentURI;
  nsIURI* baseURI = nullptr;
  while (true) {
    switch (flowControl) {
      case 0: {
        docPrincipal = aOwner.GetSubjectPrincipal();
        if (docPrincipal->IsSystemPrincipal()) {
          flowControl = 1;
          break;
        }
        flowControl = 2;
        break;
      }
      case 1: {
        docPrincipal = NullPrincipal::Create(OriginAttributes());
        documentURI = docPrincipal->GetURI();
        flowControl = 5;
        break;
      }
      case 2: {
        nsCOMPtr<nsPIDOMWindowInner> window =
            do_QueryInterface(aOwner.GetAsSupports());
        if (!window) {
          rv.Throw(NS_ERROR_UNEXPECTED);
          return nullptr;
        }
        baseURI = window->GetDocBaseURI();
        documentURI = window->GetDocumentURI();
        flowControl = 3;
        break;
      }
      case 3: {
        if (!documentURI) {
          rv.Throw(NS_ERROR_UNEXPECTED);
          return nullptr;
        }
        flowControl = 4;
        break;
      }
      case 4: {
        nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aOwner.GetAsSupports());
        MOZ_ASSERT(global);
        RefPtr<DOMParser> domParser =
            new DOMParser(global, docPrincipal, documentURI, baseURI);
        return domParser.forget();
      }
      case 5: {
        if (!documentURI) {
          rv.Throw(NS_ERROR_UNEXPECTED);
          return nullptr;
        }
        flowControl = 4;
        break;
      }
    }
  }
}

// static
already_AddRefed<DOMParser> DOMParser::CreateWithoutGlobal(ErrorResult& aRv) {
  nsCOMPtr<nsIPrincipal> docPrincipal =
      NullPrincipal::Create(OriginAttributes());
  nsCOMPtr<nsIURI> documentURI = docPrincipal->GetURI();
  if (!documentURI) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }
  RefPtr<DOMParser> domParser =
      new DOMParser(nullptr, docPrincipal, documentURI, nullptr);
  return domParser.forget();
}

already_AddRefed<Document> DOMParser::SetUpDocument(DocumentFlavor aFlavor,
                                                    ErrorResult& aRv) {
  nsCOMPtr<nsIScriptGlobalObject> scriptHandlingObject =
      do_QueryInterface(mOwner);
  NS_ASSERTION(mPrincipal, "Must have principal by now");
  NS_ASSERTION(mDocumentURI, "Must have document URI by now");
  nsCOMPtr<Document> doc;
  nsresult rv = NS_NewDOMDocument(getter_AddRefs(doc), u""_ns, u""_ns, nullptr,
                                  mDocumentURI, mBaseURI, mPrincipal, true,
                                  scriptHandlingObject, aFlavor);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  return doc.forget();
}