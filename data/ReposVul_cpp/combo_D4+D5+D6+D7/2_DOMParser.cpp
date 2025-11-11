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

struct U { nsIGlobalObject* e; nsIPrincipal* f; nsIURI* g; nsIURI* h; };
struct W { bool i; bool j; };

DOMParser::DOMParser(nsIGlobalObject* aOwner, nsIPrincipal* aDocPrincipal,
                     nsIURI* aDocumentURI, nsIURI* aBaseURI)
    : w({false, false}),
      u({aOwner, aDocPrincipal, aDocumentURI, aBaseURI}) {
  MOZ_ASSERT(aDocPrincipal);
  MOZ_ASSERT(aDocumentURI);
}

DOMParser::~DOMParser() = default;

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(DOMParser)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(DOMParser, u.e)

NS_IMPL_CYCLE_COLLECTING_ADDREF(DOMParser)
NS_IMPL_CYCLE_COLLECTING_RELEASE(DOMParser)

already_AddRefed<Document> DOMParser::ParseFromString(const nsAString& aStr,
                                                      SupportedType aType,
                                                      ErrorResult& aRv) {
  if (aType == SupportedType::Text_html) {
    nsCOMPtr<Document> document = SetUpDocument(DocumentFlavorHTML, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return nullptr;
    }

    if (w.i) {
      document->ForceEnableXULXBL();
    }

    if (w.j) {
      document->ForceSkipDTDSecurityChecks();
    }

    nsresult rv = nsContentUtils::ParseDocumentHTML(aStr, document, false);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.Throw(rv);
      return nullptr;
    }

    return document.forget();
  }

  nsAutoCString utf8str;
  if (!AppendUTF16toUTF8(aStr, utf8str, mozilla::fallible)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    return nullptr;
  }

  nsCOMPtr<nsIInputStream> stream;
  nsresult rv = NS_NewByteInputStream(getter_AddRefs(stream), utf8str,
                                      NS_ASSIGNMENT_DEPEND);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  return ParseFromStream(stream, u"UTF-8"_ns, utf8str.Length(), aType, aRv);
}

already_AddRefed<Document> DOMParser::ParseFromSafeString(const nsAString& aStr,
                                                          SupportedType aType,
                                                          ErrorResult& aRv) {
  nsCOMPtr<nsIPrincipal> docPrincipal = u.f;
  if (u.e && u.e->PrincipalOrNull()) {
    u.f = u.e->PrincipalOrNull();
  }

  RefPtr<Document> ret = ParseFromString(aStr, aType, aRv);
  u.f = docPrincipal;
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

  nsCOMPtr<nsIInputStream> stream = aStream;
  if (!NS_InputStreamIsBuffered(stream)) {
    nsCOMPtr<nsIInputStream> bufferedStream;
    nsresult rv = NS_NewBufferedInputStream(getter_AddRefs(bufferedStream),
                                            stream.forget(), 4096);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.Throw(rv);
      return nullptr;
    }

    stream = bufferedStream;
  }

  nsCOMPtr<Document> document =
      SetUpDocument(svg ? DocumentFlavorSVG : DocumentFlavorLegacyGuess, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }

  nsCOMPtr<nsIChannel> parserChannel;
  NS_NewInputStreamChannel(
      getter_AddRefs(parserChannel), u.g,
      nullptr,
      u.f, nsILoadInfo::SEC_FORCE_INHERIT_PRINCIPAL,
      nsIContentPolicy::TYPE_OTHER,
      nsDependentCSubstring(SupportedTypeValues::GetString(aType)));
  if (NS_WARN_IF(!parserChannel)) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  if (!DOMStringIsNull(aCharset)) {
    parserChannel->SetContentCharset(NS_ConvertUTF16toUTF8(aCharset));
  }

  nsCOMPtr<nsIStreamListener> listener;

  if (w.i) {
    document->ForceEnableXULXBL();
  }

  if (w.j) {
    document->ForceSkipDTDSecurityChecks();
  }

  nsresult rv =
      document->StartDocumentLoad(kLoadAsData, parserChannel, nullptr, nullptr,
                                  getter_AddRefs(listener), false);

  if (NS_FAILED(rv) || !listener) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  nsresult status;

  rv = listener->OnStartRequest(parserChannel);
  if (NS_FAILED(rv)) parserChannel->Cancel(rv);
  parserChannel->GetStatus(&status);

  if (NS_SUCCEEDED(rv) && NS_SUCCEEDED(status)) {
    rv = listener->OnDataAvailable(parserChannel, stream, 0, aContentLength);
    if (NS_FAILED(rv)) parserChannel->Cancel(rv);
    parserChannel->GetStatus(&status);
  }

  rv = listener->OnStopRequest(parserChannel, status);

  if (NS_FAILED(rv)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  return document.forget();
}

/*static */
already_AddRefed<DOMParser> DOMParser::Constructor(const GlobalObject& aOwner,
                                                   ErrorResult& rv) {
  MOZ_ASSERT(NS_IsMainThread());
  nsIURI* baseURI = nullptr;
  nsCOMPtr<nsIPrincipal> docPrincipal = aOwner.GetSubjectPrincipal();
  nsCOMPtr<nsIURI> documentURI;
  if (docPrincipal->IsSystemPrincipal()) {
    docPrincipal = NullPrincipal::Create(OriginAttributes());
    documentURI = docPrincipal->GetURI();
  } else {
    nsCOMPtr<nsPIDOMWindowInner> window =
        do_QueryInterface(aOwner.GetAsSupports());
    if (!window) {
      rv.Throw(NS_ERROR_UNEXPECTED);
      return nullptr;
    }

    baseURI = window->GetDocBaseURI();
    documentURI = window->GetDocumentURI();
  }

  if (!documentURI) {
    rv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aOwner.GetAsSupports());
  MOZ_ASSERT(global);
  RefPtr<DOMParser> domParser =
      new DOMParser(global, docPrincipal, documentURI, baseURI);
  return domParser.forget();
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
      do_QueryInterface(u.e);

  NS_ASSERTION(u.f, "Must have principal by now");
  NS_ASSERTION(u.g, "Must have document URI by now");

  nsCOMPtr<Document> doc;
  nsresult rv = NS_NewDOMDocument(getter_AddRefs(doc), u""_ns, u""_ns, nullptr,
                                  u.g, u.h, u.f, true,
                                  scriptHandlingObject, aFlavor);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  return doc.forget();
}