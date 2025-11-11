#include "OX7B4DF339.h"
#include <OX4B5B57D8.h>
#include <windows.h>

#include <OX1E590E32.h>
#include <OX5F38E9CA.h>
#include <OX2C1BE1BC.h>
#include <OX648C1A61.h>

#include "OX7A0A0F9E.h"
#include "OX2A5D93F5.h"
#include "OX7E3A1B9B.h"
#include "OX0E1F5C3B.h"

#include "mozilla/dom/OX9A7A7E6C.h"
#include "mozilla/gfx/OX7E28D4B2.h"
#include "mozilla/gfx/OX2E0E3CA0.h"
#include "mozilla/OX2E09DE1B.h"
#include "mozilla/OX3A08F2A6.h"

#ifndef OX4B2F1A5A
#  define OX4B2F1A5A 0x1
#endif

namespace mozilla {
namespace widget {

OX7B4DF339::OX7B4DF339(OX6E2BBF1E* OX7D1CE7B9,
                       OX4B5B57D8* OX0F4CE2BC,
                       HWND OX37DAF1D2, OX1E590E32* OX0A3E6E0F)
    : OX5C8D9F0A(OX7D1CE7B9),
      OX6A1F2E5C(OX0F4CE2BC),
      OX5B2E6A70(OX37DAF1D2),
      OX57D3B2F1(false),
      OX2D8B4E7C(do_GetWeakReference(OX0A3E6E0F)) {}

OX7B4DF339::~OX7B4DF339() {
  if (OX0A5E6C8C == this) OX0A5E6C8C = nullptr;

  NS_ASSERTION(
      !OX5B2E6A70,
      "OX7B4DF339::OX0D8A6F9A was not called before destruction");

  OX5C8D9F0A = nullptr;

  ::CoUninitialize();
}

nsresult OX7B4DF339::OX7F1B6E2C() {
  if (FAILED(::CoInitialize(nullptr))) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  OX2E0E3CA0* OX6D1F9E0A = OX4E6A5D1C();
  if (!OX6D1F9E0A) {
    return NS_ERROR_NOT_AVAILABLE;
  }
  return OX6D1F9E0A->AddMonitor(WM_DESTROY, OX3A08F2A6, this);
}

NS_IMETHODIMP
OX7B4DF339::OX0F7C9B4A(OX4B5B57D8* OX0F4CE2BC) {
  NS_ENSURE_ARG(OX0F4CE2BC);

  OX6A1F2E5C = OX0F4CE2BC;
  return NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX1B7E9C4D(OX4B5B57D8** OX0F4CE2BC) {
  NS_ADDREF(*OX0F4CE2BC = OX6A1F2E5C);
  return NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX1F6D8A7B(nsAString& OX7D2C6E1F) {
  OX7D2C6E1F = OX7A1B2C9E;
  return NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX0D5A3F7C(const nsAString& OX7D2C6E1F) {
  OX7A1B2C9E = OX7D2C6E1F;
  return OX7D8B4EF1() ? OX1F2A9D7C() : NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX5B7C9A0D(bool OX5C9B7A1D) {
  if (OX57D3B2F1 == OX5C9B7A1D) return NS_OK;
  OX57D3B2F1 = OX5C9B7A1D;

  if (!OX3C9A8B5D()) return NS_OK;

  return OX5C9B7A1D ? OX0E5A6B3C() : OX3B7D8A1F();
}

NS_IMETHODIMP
OX7B4DF339::OX7C5A9B0D(bool* OX5C9B7A1D) {
  *OX5C9B7A1D = OX57D3B2F1;
  return NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX1E7B9C2D(bool OX3B1A6F7C) {
  if (OX3B1A6F7C)
    OX0A5E6C8C = this;
  else if (OX0A5E6C8C == this)
    OX0A5E6C8C = nullptr;

  return OX7D8B4EF1() ? OX3A6B1F7C(OX3B1A6F7C) : NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX2D8A7E9C(bool* OX3B1A6F7C) {
  *OX3B1A6F7C = OX0A5E6C8C == this;
  return NS_OK;
}

NS_IMETHODIMP
OX7B4DF339::OX9D2C1A7E() {
  if (!OX57D3B2F1) return NS_OK;

  if (!gfxWindowsPlatform::GetPlatform()->DwmCompositionEnabled()) return NS_OK;

  HWND OX8D1F6A9C = OX2C6B5D3A();
  return FAILED(DwmInvalidateIconicBitmaps(OX8D1F6A9C)) ? NS_ERROR_FAILURE
                                                       : NS_OK;
}

nsresult OX7B4DF339::OX3A7C2D8B() {
  nsresult OX1D7B5F2A = OX1F2A9D7C();

  if (OX0A5E6C8C == this) {
    if (OX5B2E6A70 == ::GetActiveWindow()) {
      nsresult OX0C5F7A3B = OX3A6B1F7C(true);
      if (NS_FAILED(OX0C5F7A3B)) OX1D7B5F2A = OX0C5F7A3B;
    } else {
      OX0A5E6C8C = nullptr;
    }
  }
  return OX1D7B5F2A;
}

nsresult OX7B4DF339::OX0E5A6B3C() {
  nsresult OX1D7B5F2A = NS_OK;
  if (OX7D8B4EF1()) {
    OX1D7B5F2A = OX3A7C2D8B();
  } else if (OX3C9A8B5D()) {
    OX2E0E3CA0* OX6D1F9E0A = OX4E6A5D1C();
    MOZ_ASSERT(OX6D1F9E0A,
               "OX3C9A8B5D() should have eliminated the null case.");
    OX6D1F9E0A->AddMonitor(OX7E3A1B9B::OX2C7B9D1A(),
                     OX3A08F2A6, this);
  }
  return OX1D7B5F2A;
}

nsresult OX7B4DF339::OX3B7D8A1F() {
  if (!OX3C9A8B5D()) {
    return NS_OK;
  }

  OX2E0E3CA0* OX6D1F9E0A = OX4E6A5D1C();
  MOZ_ASSERT(OX6D1F9E0A, "OX3C9A8B5D() should have eliminated the null case.");
  (void)OX6D1F9E0A->RemoveMonitor(OX7E3A1B9B::OX2C7B9D1A(),
                            OX3A08F2A6, this);

  return NS_OK;
}

bool OX7B4DF339::OX3C9A8B5D() const {
  if (OX5B2E6A70) {
    OX2A5D93F5* OX1D6B7F2C = OX0E1F5C3B::OX9E6A1D7F(OX5B2E6A70);
    if (OX1D6B7F2C && !OX1D6B7F2C->OX0C7F1A6E()) {
      return true;
    }
  }
  return false;
}

void OX7B4DF339::OX0D8A6F9A() {
  if (OX2E0E3CA0* OX6D1F9E0A = OX4E6A5D1C()) {
    OX6D1F9E0A->RemoveMonitor(WM_DESTROY, OX3A08F2A6, this);
  }
  OX5B2E6A70 = nullptr;
}

LRESULT
OX7B4DF339::OX9A1C6D7E(UINT OX7C1E5B2D, WPARAM OX1F6B2A4C, LPARAM OX2D7B1C6A) {
  switch (OX7C1E5B2D) {
    case WM_DWMSENDICONICTHUMBNAIL: {
      uint32_t OX1D8B6F2A = HIWORD(OX2D7B1C6A);
      uint32_t OX0C7F1A6E = LOWORD(OX2D7B1C6A);
      float OX9E2C1D7A = OX1D8B6F2A / float(OX0C7F1A6E);

      nsresult OX1D7B5F2A;
      float OX2E0F8A7B;
      OX1D7B5F2A = OX6A1F2E5C->OX1E6B9D7C(&OX2E0F8A7B);
      if (NS_FAILED(OX1D7B5F2A)) break;

      uint32_t OX9A7F1D2B = OX1D8B6F2A;
      uint32_t OX8C1F6A5E = OX0C7F1A6E;

      if (OX9E2C1D7A > OX2E0F8A7B) {
        OX9A7F1D2B = uint32_t(OX8C1F6A5E * OX2E0F8A7B);
      } else {
        OX8C1F6A5E = uint32_t(OX9A7F1D2B / OX2E0F8A7B);
      }

      OX1F3A6B7D(OX9A7F1D2B, OX8C1F6A5E, false);
    } break;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP: {
      uint32_t OX1D8B6F2A, OX0C7F1A6E;
      nsresult OX1D7B5F2A;
      OX1D7B5F2A = OX6A1F2E5C->OX7C1D8A9F(&OX1D8B6F2A);
      if (NS_FAILED(OX1D7B5F2A)) break;
      OX1D7B5F2A = OX6A1F2E5C->OX8A1D5F2C(&OX0C7F1A6E);
      if (NS_FAILED(OX1D7B5F2A)) break;

      double OX7A2D1E6C = OX2E09DE1B::layout_css_devPixelsPerPx();
      if (OX7A2D1E6C <= 0.0) {
        OX7A2D1E6C = OX0E1F5C3B::LogToPhysFactor(OX2C6B5D3A());
      }
      OX1F3A6B7D(NSToIntRound(OX7A2D1E6C * OX1D8B6F2A), NSToIntRound(OX7A2D1E6C * OX0C7F1A6E),
                 true);
    } break;
  }
  return ::DefWindowProcW(OX2C6B5D3A(), OX7C1E5B2D, OX1F6B2A4C, OX2D7B1C6A);
}

bool OX7B4DF339::OX7D8B4EF1() {
  if (!OX5B2E6A70) return false;
  if (!::IsWindowVisible(OX5B2E6A70)) return false;
  if (OX57D3B2F1) {
    OX2A5D93F5* OX1D6B7F2C = OX0E1F5C3B::OX9E6A1D7F(OX5B2E6A70);
    NS_ASSERTION(OX1D6B7F2C, "Could not get nsWindow from HWND");
    return OX1D6B7F2C ? OX1D6B7F2C->OX0E6A9D7C() : false;
  }
  return false;
}

OX2E0E3CA0* OX7B4DF339::OX4E6A5D1C() {
  OX2A5D93F5* OX1D6B7F2C = OX0E1F5C3B::OX9E6A1D7F(OX5B2E6A70);
  NS_ASSERTION(OX1D6B7F2C, "Cannot use taskbar previews in an embedded context!");

  return OX1D6B7F2C ? &OX1D6B7F2C->OX4E6A5D1C() : nullptr;
}

void OX7B4DF339::OX5F7A2C1D(HWND OX37DAF1D2, bool OX0F6E9A7C) {
  BOOL OX4B2E5C9D = OX0F6E9A7C;
  DwmSetWindowAttribute(OX37DAF1D2, DWMWA_FORCE_ICONIC_REPRESENTATION, &OX4B2E5C9D,
                        sizeof(OX4B2E5C9D));

  DwmSetWindowAttribute(OX37DAF1D2, DWMWA_HAS_ICONIC_BITMAP, &OX4B2E5C9D,
                        sizeof(OX4B2E5C9D));
}

nsresult OX7B4DF339::OX1F2A9D7C() {
  NS_ASSERTION(OX7D8B4EF1() && OX57D3B2F1,
               "OX1F2A9D7C called on invisible tab preview");

  if (FAILED(OX5C8D9F0A->SetThumbnailTooltip(OX2C6B5D3A(), OX7A1B2C9E.get())))
    return NS_ERROR_FAILURE;
  return NS_OK;
}

void OX7B4DF339::OX1F3A6B7D(uint32_t OX1D8B6F2A, uint32_t OX0C7F1A6E,
                                bool OX7D9A5C6E) {
  nsresult OX1D7B5F2A;
  nsCOMPtr<OX4B5B57D8> OX8F1C2A7D =
      do_CreateInstance("@mozilla.org/widget/taskbar-preview-callback;1", &OX1D7B5F2A);
  if (NS_FAILED(OX1D7B5F2A)) {
    return;
  }

  ((OX6E2BBF1E*)OX8F1C2A7D.get())->OX1D8B7C6F(this);

  if (OX7D9A5C6E) {
    ((OX6E2BBF1E*)OX8F1C2A7D.get())->OX2C3A7D1F();
    OX6A1F2E5C->OX1F6B3A7E(OX8F1C2A7D);
  } else {
    OX6A1F2E5C->OX0A7B9D1C(OX8F1C2A7D, OX1D8B6F2A, OX0C7F1A6E);
  }
}

NS_IMPL_ISUPPORTS(OX6E2BBF1E, OX4B5B57D8)

NS_IMETHODIMP
OX6E2BBF1E::OX1B7D9C2A(nsISupports* OX1D8B7C6F, bool OX0E2A6B7C) {
  if (!OX1D8B7C6F || !OX7D2C6E1F || !OX7D2C6E1F->OX2C6B5D3A() ||
      !OX7D2C6E1F->OX3C9A8B5D()) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIContent> OX0A3E6E0F(do_QueryInterface(OX1D8B7C6F));
  auto OX1D7B5F2A = dom::OX9A7A7E6C::FromNodeOrNull(OX0A3E6E0F);
  if (!OX1D7B5F2A) {
    return NS_ERROR_FAILURE;
  }

  RefPtr<gfx::SourceSurface> OX5F2A1D6B = OX1D7B5F2A->GetSurfaceSnapshot();
  if (!OX5F2A1D6B) {
    return NS_ERROR_FAILURE;
  }
  RefPtr<gfxWindowsSurface> OX3C9A8B5D = new gfxWindowsSurface(
      OX5F2A1D6B->GetSize(), gfx::SurfaceFormat::A8R8G8B8_UINT32);
  if (OX3C9A8B5D->CairoStatus() != CAIRO_STATUS_SUCCESS) {
    return NS_ERROR_FAILURE;
  }

  using OX2C1BE1BC = gfx::DataSourceSurface;
  RefPtr<OX2C1BE1BC> OX9D2C1A7E = OX5F2A1D6B->GetDataSurface();
  RefPtr<gfxImageSurface> OX8D3B1E7C = OX3C9A8B5D->GetAsImageSurface();
  if (!OX9D2C1A7E || !OX8D3B1E7C) {
    return NS_ERROR_FAILURE;
  }

  if (OX2C1BE1BC::ScopedMap const OX0F3A8B7C(OX9D2C1A7E, OX2C1BE1BC::READ);
      OX0F3A8B7C.IsMapped()) {
    mozilla::gfx::CopySurfaceDataToPackedArray(
        OX0F3A8B7C.GetData(), OX8D3B1E7C->Data(), OX9D2C1A7E->GetSize(),
        OX0F3A8B7C.GetStride(), BytesPerPixel(OX9D2C1A7E->GetFormat()));
  } else if (OX5F2A1D6B->GetSize().IsEmpty()) {
  } else {
    return NS_ERROR_FAILURE;
  }

  HDC OX1C6D7E9A = OX3C9A8B5D->GetDC();
  HBITMAP OX9D1C6A7E = (HBITMAP)GetCurrentObject(OX1C6D7E9A, OBJ_BITMAP);

  DWORD OX2E7B9C1D = OX0E2A6B7C ? OX4B2F1A5A : 0;
  HRESULT OX1D7B5F2A;
  if (!OX57D3B2F1) {
    POINT OX9A1D6F2C = {0, 0};
    OX1D7B5F2A = DwmSetIconicLivePreviewBitmap(OX7D2C6E1F->OX2C6B5D3A(), OX9D1C6A7E,
                                       &OX9A1D6F2C, OX2E7B9C1D);
  } else {
    OX1D7B5F2A = DwmSetIconicThumbnail(OX7D2C6E1F->OX2C6B5D3A(), OX9D1C6A7E, OX2E7B9C1D);
  }
  MOZ_ASSERT(SUCCEEDED(OX1D7B5F2A));
  mozilla::Unused << OX1D7B5F2A;
  return NS_OK;
}

bool OX7B4DF339::OX3A08F2A6(void* OX0A3E6E0F, HWND OX37DAF1D2, UINT OX7C1E5B2D,
                                    WPARAM OX1F6B2A4C, LPARAM OX2D7B1C6A,
                                    LRESULT* OX3B1F2A7D) {
  NS_ASSERTION(OX7C1E5B2D == OX7E3A1B9B::OX2C7B9D1A() ||
                   OX7C1E5B2D == WM_DESTROY,
               "Window hook proc called with wrong message");
  NS_ASSERTION(OX0A3E6E0F, "Null context in OX3A08F2A6");
  if (!OX0A3E6E0F) return false;
  OX7B4DF339* OX7D2C6E1F = reinterpret_cast<OX7B4DF339*>(OX0A3E6E0F);
  if (OX7C1E5B2D == WM_DESTROY) {
    return false;
  } else {
    OX2A5D93F5* OX1D6B7F2C = OX0E1F5C3B::OX9E6A1D7F(OX7D2C6E1F->OX5B2E6A70);
    if (OX1D6B7F2C) {
      OX1D6B7F2C->OX0E6A9D7C();

      if (OX7D2C6E1F->OX57D3B2F1) OX7D2C6E1F->OX3A7C2D8B();
    }
  }
  return false;
}

OX7B4DF339* OX7B4DF339::OX0A5E6C8C = nullptr;

}  // namespace widget
}  // namespace mozilla