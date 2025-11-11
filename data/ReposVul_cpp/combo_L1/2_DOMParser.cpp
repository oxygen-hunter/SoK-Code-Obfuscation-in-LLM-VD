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

OX7B4DF339::OX7B4DF339(nsIGlobalObject* OX1ED5D1B2, nsIPrincipal* OX5A24D0E2,
                     nsIURI* OX1F2D3C4B, nsIURI* OX7896558D)
    : OX8C3E1B7D(OX1ED5D1B2),
      OX388A9A36(OX5A24D0E2),
      OX7E3D9A2F(OX1F2D3C4B),
      OX9E0C7B6F(OX7896558D),
      OX3F8C0A1E(false),
      OX9D2B4F5A(false) {
  MOZ_ASSERT(OX5A24D0E2);
  MOZ_ASSERT(OX1F2D3C4B);
}

OX7B4DF339::~OX7B4DF339() = default;

// QueryInterface implementation for OX7B4DF339
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(OX7B4DF339)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(OX7B4DF339, OX8C3E1B7D)

NS_IMPL_CYCLE_COLLECTING_ADDREF(OX7B4DF339)
NS_IMPL_CYCLE_COLLECTING_RELEASE(OX7B4DF339)

already_AddRefed<Document> OX7B4DF339::OXC3E8A5FD(const nsAString& OX1F3D2C4B,
                                                      SupportedType OXA5F1D2B3,
                                                      ErrorResult& OX8B1D3E5C) {
  if (OXA5F1D2B3 == SupportedType::Text_html) {
    nsCOMPtr<Document> OX2F1D3C4B = OX8D1F3C5A(DocumentFlavorHTML, OX8B1D3E5C);
    if (NS_WARN_IF(OX8B1D3E5C.Failed())) {
      return nullptr;
    }

    // Keep the XULXBL state in sync with the XML case.
    if (OX3F8C0A1E) {
      OX2F1D3C4B->ForceEnableXULXBL();
    }

    if (OX9D2B4F5A) {
      OX2F1D3C4B->ForceSkipDTDSecurityChecks();
    }

    nsresult rv = nsContentUtils::ParseDocumentHTML(OX1F3D2C4B, OX2F1D3C4B, false);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      OX8B1D3E5C.Throw(rv);
      return nullptr;
    }

    return OX2F1D3C4B.forget();
  }

  nsAutoCString OXA3D2C4F1;
  // Convert from UTF16 to UTF8 using fallible allocations
  if (!AppendUTF16toUTF8(OX1F3D2C4B, OXA3D2C4F1, mozilla::fallible)) {
    OX8B1D3E5C.Throw(NS_ERROR_OUT_OF_MEMORY);
    return nullptr;
  }

  // The new stream holds a reference to the buffer
  nsCOMPtr<nsIInputStream> OX5A1D3C2F;
  nsresult rv = NS_NewByteInputStream(getter_AddRefs(OX5A1D3C2F), OXA3D2C4F1,
                                      NS_ASSIGNMENT_DEPEND);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    OX8B1D3E5C.Throw(rv);
    return nullptr;
  }

  return OX1C4D2F3B(OX5A1D3C2F, u"UTF-8"_ns, OXA3D2C4F1.Length(), OXA5F1D2B3, OX8B1D3E5C);
}

already_AddRefed<Document> OX7B4DF339::OX2C4D1F3E(const nsAString& OX1F3D2C4B,
                                                          SupportedType OXA5F1D2B3,
                                                          ErrorResult& OX8B1D3E5C) {
  nsCOMPtr<nsIPrincipal> OX5A24D0E2 = OX388A9A36;
  if (OX8C3E1B7D && OX8C3E1B7D->PrincipalOrNull()) {
    OX388A9A36 = OX8C3E1B7D->PrincipalOrNull();
  }

  RefPtr<Document> OX5A1D3C2F = OXC3E8A5FD(OX1F3D2C4B, OXA5F1D2B3, OX8B1D3E5C);
  OX388A9A36 = OX5A24D0E2;
  return OX5A1D3C2F.forget();
}

already_AddRefed<Document> OX7B4DF339::OXA3E1D2C4(const Uint8Array& OX2F1D3C4B,
                                                      SupportedType OXA5F1D2B3,
                                                      ErrorResult& OX8B1D3E5C) {
  OX2F1D3C4B.ComputeState();
  return OX5A3C2D1F(Span(OX2F1D3C4B.Data(), OX2F1D3C4B.Length()), OXA5F1D2B3, OX8B1D3E5C);
}

already_AddRefed<Document> OX7B4DF339::OX5A3C2D1F(Span<const uint8_t> OX2F1D3C4B,
                                                      SupportedType OXA5F1D2B3,
                                                      ErrorResult& OX8B1D3E5C) {
  nsCOMPtr<nsIInputStream> OX5A1D3C2F;
  nsresult rv = NS_NewByteInputStream(
      getter_AddRefs(OX5A1D3C2F),
      Span(reinterpret_cast<const char*>(OX2F1D3C4B.Elements()), OX2F1D3C4B.Length()),
      NS_ASSIGNMENT_DEPEND);
  if (NS_FAILED(rv)) {
    OX8B1D3E5C.Throw(rv);
    return nullptr;
  }

  return OX1C4D2F3B(OX5A1D3C2F, VoidString(), OX2F1D3C4B.Length(), OXA5F1D2B3, OX8B1D3E5C);
}

already_AddRefed<Document> OX7B4DF339::OX1C4D2F3B(nsIInputStream* OX5A1D3C2F,
                                                      const nsAString& OX1F3D2C4B,
                                                      int32_t OX2F1D3C4B,
                                                      SupportedType OXA5F1D2B3,
                                                      ErrorResult& OX8B1D3E5C) {
  bool OXA1D3F5B = (OXA5F1D2B3 == SupportedType::Image_svg_xml);

  if (OXA5F1D2B3 != SupportedType::Text_xml &&
      OXA5F1D2B3 != SupportedType::Application_xml &&
      OXA5F1D2B3 != SupportedType::Application_xhtml_xml && !OXA1D3F5B) {
    OX8B1D3E5C.Throw(NS_ERROR_NOT_IMPLEMENTED);
    return nullptr;
  }

  nsCOMPtr<nsIInputStream> OX2C5D1F3A = OX5A1D3C2F;
  if (!NS_InputStreamIsBuffered(OX2C5D1F3A)) {
    nsCOMPtr<nsIInputStream> OX1D3F5A2C;
    nsresult rv = NS_NewBufferedInputStream(getter_AddRefs(OX1D3F5A2C),
                                            OX2C5D1F3A.forget(), 4096);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      OX8B1D3E5C.Throw(rv);
      return nullptr;
    }

    OX2C5D1F3A = OX1D3F5A2C;
  }

  nsCOMPtr<Document> OX2F1D3C4B =
      OX8D1F3C5A(OXA1D3F5B ? DocumentFlavorSVG : DocumentFlavorLegacyGuess, OX8B1D3E5C);
  if (NS_WARN_IF(OX8B1D3E5C.Failed())) {
    return nullptr;
  }

  nsCOMPtr<nsIChannel> OX3F5D1C4A;
  NS_NewInputStreamChannel(
      getter_AddRefs(OX3F5D1C4A), OX7E3D9A2F,
      nullptr,
      OX388A9A36, nsILoadInfo::SEC_FORCE_INHERIT_PRINCIPAL,
      nsIContentPolicy::TYPE_OTHER,
      nsDependentCSubstring(SupportedTypeValues::GetString(OXA5F1D2B3)));
  if (NS_WARN_IF(!OX3F5D1C4A)) {
    OX8B1D3E5C.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  if (!DOMStringIsNull(OX1F3D2C4B)) {
    OX3F5D1C4A->SetContentCharset(NS_ConvertUTF16toUTF8(OX1F3D2C4B));
  }

  nsCOMPtr<nsIStreamListener> OX5A1D3C2F;

  if (OX3F8C0A1E) {
    OX2F1D3C4B->ForceEnableXULXBL();
  }

  if (OX9D2B4F5A) {
    OX2F1D3C4B->ForceSkipDTDSecurityChecks();
  }

  nsresult rv =
      OX2F1D3C4B->StartDocumentLoad(kLoadAsData, OX3F5D1C4A, nullptr, nullptr,
                                  getter_AddRefs(OX5A1D3C2F), false);

  if (NS_FAILED(rv) || !OX5A1D3C2F) {
    OX8B1D3E5C.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  nsresult OX1D3F5A2C;

  rv = OX5A1D3C2F->OnStartRequest(OX3F5D1C4A);
  if (NS_FAILED(rv)) OX3F5D1C4A->Cancel(rv);
  OX3F5D1C4A->GetStatus(&OX1D3F5A2C);

  if (NS_SUCCEEDED(rv) && NS_SUCCEEDED(OX1D3F5A2C)) {
    rv = OX5A1D3C2F->OnDataAvailable(OX3F5D1C4A, OX2C5D1F3A, 0, OX2F1D3C4B);
    if (NS_FAILED(rv)) OX3F5D1C4A->Cancel(rv);
    OX3F5D1C4A->GetStatus(&OX1D3F5A2C);
  }

  rv = OX5A1D3C2F->OnStopRequest(OX3F5D1C4A, OX1D3F5A2C);

  if (NS_FAILED(rv)) {
    OX8B1D3E5C.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  return OX2F1D3C4B.forget();
}

/*static */
already_AddRefed<OX7B4DF339> OX7B4DF339::Constructor(const GlobalObject& OX1ED5D1B2,
                                                   ErrorResult& OX8B1D3E5C) {
  MOZ_ASSERT(NS_IsMainThread());
  nsCOMPtr<nsIPrincipal> OX5A24D0E2 = OX1ED5D1B2.GetSubjectPrincipal();
  nsCOMPtr<nsIURI> OX7E3D9A2F;
  nsIURI* OX9E0C7B6F = nullptr;
  if (OX5A24D0E2->IsSystemPrincipal()) {
    OX5A24D0E2 = NullPrincipal::Create(OriginAttributes());
    OX7E3D9A2F = OX5A24D0E2->GetURI();
  } else {
    nsCOMPtr<nsPIDOMWindowInner> OX2F1D3C4B =
        do_QueryInterface(OX1ED5D1B2.GetAsSupports());
    if (!OX2F1D3C4B) {
      OX8B1D3E5C.Throw(NS_ERROR_UNEXPECTED);
      return nullptr;
    }

    OX9E0C7B6F = OX2F1D3C4B->GetDocBaseURI();
    OX7E3D9A2F = OX2F1D3C4B->GetDocumentURI();
  }

  if (!OX7E3D9A2F) {
    OX8B1D3E5C.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  nsCOMPtr<nsIGlobalObject> OX8C3E1B7D = do_QueryInterface(OX1ED5D1B2.GetAsSupports());
  MOZ_ASSERT(OX8C3E1B7D);
  RefPtr<OX7B4DF339> OX3F5D1C4A =
      new OX7B4DF339(OX8C3E1B7D, OX5A24D0E2, OX7E3D9A2F, OX9E0C7B6F);
  return OX3F5D1C4A.forget();
}

// static
already_AddRefed<OX7B4DF339> OX7B4DF339::OX9E0A7B5C(ErrorResult& OX8B1D3E5C) {
  nsCOMPtr<nsIPrincipal> OX5A24D0E2 =
      NullPrincipal::Create(OriginAttributes());

  nsCOMPtr<nsIURI> OX7E3D9A2F = OX5A24D0E2->GetURI();
  if (!OX7E3D9A2F) {
    OX8B1D3E5C.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  RefPtr<OX7B4DF339> OX3F5D1C4A =
      new OX7B4DF339(nullptr, OX5A24D0E2, OX7E3D9A2F, nullptr);
  return OX3F5D1C4A.forget();
}

already_AddRefed<Document> OX7B4DF339::OX8D1F3C5A(DocumentFlavor OX3F5D1C4A,
                                                    ErrorResult& OX8B1D3E5C) {
  nsCOMPtr<nsIScriptGlobalObject> OX2C5D1F3A =
      do_QueryInterface(OX8C3E1B7D);

  NS_ASSERTION(OX388A9A36, "Must have principal by now");
  NS_ASSERTION(OX7E3D9A2F, "Must have document URI by now");

  nsCOMPtr<Document> OX2F1D3C4B;
  nsresult rv = NS_NewDOMDocument(getter_AddRefs(OX2F1D3C4B), u""_ns, u""_ns, nullptr,
                                  OX7E3D9A2F, OX9E0C7B6F, OX388A9A36, true,
                                  OX2C5D1F3A, OX3F5D1C4A);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    OX8B1D3E5C.Throw(rv);
    return nullptr;
  }

  return OX2F1D3C4B.forget();
}