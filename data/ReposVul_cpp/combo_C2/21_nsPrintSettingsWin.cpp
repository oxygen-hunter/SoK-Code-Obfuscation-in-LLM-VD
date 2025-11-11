#include "nsPrintSettingsWin.h"

#include "mozilla/ArrayUtils.h"
#include "nsCRT.h"
#include "nsDeviceContextSpecWin.h"
#include "nsPrintSettingsImpl.h"
#include "WinUtils.h"

using namespace mozilla;

const short kPaperSizeUnits[] = {
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeInches,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
    nsIPrintSettings::kPaperSizeMillimeters,
};

NS_IMPL_ISUPPORTS_INHERITED(nsPrintSettingsWin, nsPrintSettings,
                            nsIPrintSettingsWin)

nsPrintSettingsWin::nsPrintSettingsWin()
    : nsPrintSettings(),
      mDeviceName(nullptr),
      mDriverName(nullptr),
      mDevMode(nullptr) {}

nsPrintSettingsWin::nsPrintSettingsWin(const nsPrintSettingsWin& aPS)
    : mDevMode(nullptr) {
  *this = aPS;
}

void nsPrintSettingsWin::PaperSizeUnitFromDmPaperSize(short aPaperSize,
                                                      int16_t& aPaperSizeUnit) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (aPaperSize > 0 && aPaperSize < int32_t(ArrayLength(kPaperSizeUnits))) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        aPaperSizeUnit = kPaperSizeUnits[aPaperSize];
        dispatcher = 2;
        break;
      case 2:
        return;
    }
  }
}

void nsPrintSettingsWin::InitWithInitializer(
    const PrintSettingsInitializer& aSettings) {
  nsPrintSettings::InitWithInitializer(aSettings);

  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (aSettings.mDevmodeWStorage.Length() < sizeof(DEVMODEW)) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        return;
      case 2:
        {
          auto* devmode =
              reinterpret_cast<const DEVMODEW*>(aSettings.mDevmodeWStorage.Elements());
          dispatcher = 3;
        }
        break;
      case 3:
        {
          auto* devmode =
              reinterpret_cast<const DEVMODEW*>(aSettings.mDevmodeWStorage.Elements());
          if (devmode->dmSize != sizeof(DEVMODEW) ||
              devmode->dmSize + devmode->dmDriverExtra >
                  aSettings.mDevmodeWStorage.Length()) {
            dispatcher = 4;
            break;
          }
          dispatcher = 5;
        }
        break;
      case 4:
        return;
      case 5:
        {
          auto* devmode =
              reinterpret_cast<const DEVMODEW*>(aSettings.mDevmodeWStorage.Elements());
          SetDevMode(const_cast<DEVMODEW*>(devmode));
          dispatcher = 6;
        }
        break;
      case 6:
        if (mDevMode->dmFields & DM_SCALE) {
          dispatcher = 7;
          break;
        }
        dispatcher = 9;
        break;
      case 7:
        {
          double scale = double(mDevMode->dmScale) / 100.0f;
          dispatcher = 8;
        }
        break;
      case 8:
        {
          double scale = double(mDevMode->dmScale) / 100.0f;
          if (mScaling == 1.0 || scale != 1.0) {
            SetScaling(scale);
          }
          mDevMode->dmScale = 100;
          dispatcher = 9;
        }
        break;
      case 9:
        return;
    }
  }
}

already_AddRefed<nsIPrintSettings> CreatePlatformPrintSettings(
    const PrintSettingsInitializer& aSettings) {
  RefPtr<nsPrintSettings> settings = aSettings.mPrintSettings.get();
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!settings) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        settings = MakeRefPtr<nsPrintSettingsWin>();
        dispatcher = 2;
        break;
      case 2:
        settings->InitWithInitializer(aSettings);
        dispatcher = 3;
        break;
      case 3:
        return settings.forget();
    }
  }
}

nsPrintSettingsWin::~nsPrintSettingsWin() {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (mDevMode) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        ::HeapFree(::GetProcessHeap(), 0, mDevMode);
        dispatcher = 2;
        break;
      case 2:
        return;
    }
  }
}

NS_IMETHODIMP nsPrintSettingsWin::SetDeviceName(const nsAString& aDeviceName) {
  mDeviceName = aDeviceName;
  return NS_OK;
}
NS_IMETHODIMP nsPrintSettingsWin::GetDeviceName(nsAString& aDeviceName) {
  aDeviceName = mDeviceName;
  return NS_OK;
}

NS_IMETHODIMP nsPrintSettingsWin::SetDriverName(const nsAString& aDriverName) {
  mDriverName = aDriverName;
  return NS_OK;
}
NS_IMETHODIMP nsPrintSettingsWin::GetDriverName(nsAString& aDriverName) {
  aDriverName = mDriverName;
  return NS_OK;
}

void nsPrintSettingsWin::CopyDevMode(DEVMODEW* aInDevMode,
                                     DEVMODEW*& aOutDevMode) {
  aOutDevMode = nullptr;
  size_t size = aInDevMode->dmSize + aInDevMode->dmDriverExtra;
  aOutDevMode =
      (LPDEVMODEW)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (aOutDevMode) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        memcpy(aOutDevMode, aInDevMode, size);
        dispatcher = 2;
        break;
      case 2:
        return;
    }
  }
}

NS_IMETHODIMP nsPrintSettingsWin::GetDevMode(DEVMODEW** aDevMode) {
  NS_ENSURE_ARG_POINTER(aDevMode);

  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (mDevMode) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        CopyDevMode(mDevMode, *aDevMode);
        dispatcher = 3;
        break;
      case 2:
        *aDevMode = nullptr;
        dispatcher = 3;
        break;
      case 3:
        return NS_OK;
    }
  }
}

NS_IMETHODIMP nsPrintSettingsWin::SetDevMode(DEVMODEW* aDevMode) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (mDevMode) {
          dispatcher = 1;
          break;
        }
        dispatcher = 3;
        break;
      case 1:
        ::HeapFree(::GetProcessHeap(), 0, mDevMode);
        dispatcher = 2;
        break;
      case 2:
        mDevMode = nullptr;
        dispatcher = 3;
        break;
      case 3:
        if (aDevMode) {
          dispatcher = 4;
          break;
        }
        dispatcher = 5;
        break;
      case 4:
        CopyDevMode(aDevMode, mDevMode);
        dispatcher = 5;
        break;
      case 5:
        return NS_OK;
    }
  }
}

void nsPrintSettingsWin::InitUnwriteableMargin(HDC aHdc) {
  mozilla::gfx::MarginDouble margin =
      mozilla::widget::WinUtils::GetUnwriteableMarginsForDeviceInInches(aHdc);

  mUnwriteableMargin.SizeTo(NS_INCHES_TO_INT_TWIPS(margin.top),
                            NS_INCHES_TO_INT_TWIPS(margin.right),
                            NS_INCHES_TO_INT_TWIPS(margin.bottom),
                            NS_INCHES_TO_INT_TWIPS(margin.left));
}

void nsPrintSettingsWin::CopyFromNative(HDC aHdc, DEVMODEW* aDevMode) {
  MOZ_ASSERT(aHdc);
  MOZ_ASSERT(aDevMode);

  mIsInitedFromPrinter = true;
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (aDevMode->dmFields & DM_ORIENTATION) {
          dispatcher = 1;
          break;
        }
        dispatcher = 4;
        break;
      case 1:
        {
          const bool areSheetsOfPaperPortraitMode =
              (aDevMode->dmOrientation == DMORIENT_PORTRAIT);
          dispatcher = 2;
        }
        break;
      case 2:
        {
          const bool areSheetsOfPaperPortraitMode =
              (aDevMode->dmOrientation == DMORIENT_PORTRAIT);
          const bool arePagesPortraitMode =
              (areSheetsOfPaperPortraitMode != HasOrthogonalSheetsAndPages());

          mOrientation = int32_t(arePagesPortraitMode ? kPortraitOrientation
                                                      : kLandscapeOrientation);
          dispatcher = 3;
        }
        break;
      case 3:
        {
          const bool areSheetsOfPaperPortraitMode =
              (aDevMode->dmOrientation == DMORIENT_PORTRAIT);
          const bool arePagesPortraitMode =
              (areSheetsOfPaperPortraitMode != HasOrthogonalSheetsAndPages());

          mOrientation = int32_t(arePagesPortraitMode ? kPortraitOrientation
                                                      : kLandscapeOrientation);
          dispatcher = 4;
        }
        break;
      case 4:
        if (aDevMode->dmFields & DM_COPIES) {
          dispatcher = 5;
          break;
        }
        dispatcher = 6;
        break;
      case 5:
        mNumCopies = aDevMode->dmCopies;
        dispatcher = 6;
        break;
      case 6:
        if (aDevMode->dmFields & DM_DUPLEX) {
          dispatcher = 7;
          break;
        }
        dispatcher = 17;
        break;
      case 7:
        switch (aDevMode->dmDuplex) {
          default:
            dispatcher = 8;
            break;
          case DMDUP_SIMPLEX:
            dispatcher = 9;
            break;
          case DMDUP_VERTICAL:
            dispatcher = 11;
            break;
          case DMDUP_HORIZONTAL:
            dispatcher = 13;
            break;
        }
        break;
      case 8:
        MOZ_FALLTHROUGH_ASSERT("bad value for dmDuplex field");
        dispatcher = 9;
        break;
      case 9:
        mDuplex = kDuplexNone;
        dispatcher = 10;
        break;
      case 10:
        dispatcher = 17;
        break;
      case 11:
        mDuplex = kDuplexFlipOnLongEdge;
        dispatcher = 12;
        break;
      case 12:
        dispatcher = 17;
        break;
      case 13:
        mDuplex = kDuplexFlipOnShortEdge;
        dispatcher = 14;
        break;
      case 14:
        dispatcher = 17;
        break;
      case 15:
        dispatcher = 17;
        break;
      case 16:
        dispatcher = 17;
        break;
      case 17:
        if (aDevMode->dmFields & DM_SCALE) {
          dispatcher = 18;
          break;
        }
        dispatcher = 21;
        break;
      case 18:
        {
          double scale = double(aDevMode->dmScale) / 100.0f;
          dispatcher = 19;
        }
        break;
      case 19:
        {
          double scale = double(aDevMode->dmScale) / 100.0f;
          if (mScaling == 1.0 || scale != 1.0) {
            mScaling = scale;
          }
          dispatcher = 20;
        }
        break;
      case 20:
        aDevMode->dmScale = 100;
        dispatcher = 21;
        break;
      case 21:
        if (aDevMode->dmFields & DM_PAPERSIZE) {
          dispatcher = 22;
          break;
        }
        dispatcher = 25;
        break;
      case 22:
        mPaperId.Truncate(0);
        dispatcher = 23;
        break;
      case 23:
        mPaperId.AppendInt(aDevMode->dmPaperSize);
        dispatcher = 24;
        break;
      case 24:
        PaperSizeUnitFromDmPaperSize(aDevMode->dmPaperSize, mPaperSizeUnit);
        dispatcher = 25;
        break;
      case 25:
        if (aDevMode->dmFields & DM_COLOR) {
          dispatcher = 26;
          break;
        }
        dispatcher = 27;
        break;
      case 26:
        mPrintInColor = aDevMode->dmColor == DMCOLOR_COLOR;
        dispatcher = 27;
        break;
      case 27:
        InitUnwriteableMargin(aHdc);
        dispatcher = 28;
        break;
      case 28:
        int pixelsPerInchY = ::GetDeviceCaps(aHdc, LOGPIXELSY);
        dispatcher = 29;
        break;
      case 29:
        int physicalHeight = ::GetDeviceCaps(aHdc, PHYSICALHEIGHT);
        dispatcher = 30;
        break;
      case 30:
        double physicalHeightInch = double(physicalHeight) / pixelsPerInchY;
        dispatcher = 31;
        break;
      case 31:
        int pixelsPerInchX = ::GetDeviceCaps(aHdc, LOGPIXELSX);
        dispatcher = 32;
        break;
      case 32:
        int physicalWidth = ::GetDeviceCaps(aHdc, PHYSICALWIDTH);
        dispatcher = 33;
        break;
      case 33:
        double physicalWidthInch = double(physicalWidth) / pixelsPerInchX;
        dispatcher = 34;
        break;
      case 34:
        double paperHeightInch = mOrientation == kPortraitOrientation
                                     ? physicalHeightInch
                                     : physicalWidthInch;
        dispatcher = 35;
        break;
      case 35:
        mPaperHeight = mPaperSizeUnit == kPaperSizeInches
                           ? paperHeightInch
                           : paperHeightInch * MM_PER_INCH_FLOAT;
        dispatcher = 36;
        break;
      case 36:
        double paperWidthInch = mOrientation == kPortraitOrientation
                                    ? physicalWidthInch
                                    : physicalHeightInch;
        dispatcher = 37;
        break;
      case 37:
        mPaperWidth = mPaperSizeUnit == kPaperSizeInches
                          ? paperWidthInch
                          : paperWidthInch * MM_PER_INCH_FLOAT;
        dispatcher = 38;
        break;
      case 38:
        mResolution = pixelsPerInchY;
        dispatcher = 39;
        break;
      case 39:
        return;
    }
  }
}

void nsPrintSettingsWin::CopyToNative(DEVMODEW* aDevMode) {
  MOZ_ASSERT(aDevMode);

  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!mPaperId.IsEmpty()) {
          dispatcher = 1;
          break;
        }
        dispatcher = 5;
        break;
      case 1:
        aDevMode->dmPaperSize = _wtoi((const wchar_t*)mPaperId.BeginReading());
        dispatcher = 2;
        break;
      case 2:
        aDevMode->dmFields |= DM_PAPERSIZE;
        dispatcher = 3;
        break;
      case 3:
        dispatcher = 5;
        break;
      case 4:
        dispatcher = 5;
        break;
      case 5:
        aDevMode->dmFields |= DM_COLOR;
        dispatcher = 6;
        break;
      case 6:
        aDevMode->dmColor = mPrintInColor ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;
        dispatcher = 7;
        break;
      case 7:
        double tenthsOfAmmPerSizeUnit =
            mPaperSizeUnit == kPaperSizeInches ? MM_PER_INCH_FLOAT * 10.0 : 10.0;
        dispatcher = 8;
        break;
      case 8:
        if (mPaperHeight > 0) {
          dispatcher = 9;
          break;
        }
        dispatcher = 12;
        break;
      case 9:
        aDevMode->dmPaperLength = std::round(mPaperHeight * tenthsOfAmmPerSizeUnit);
        dispatcher = 10;
        break;
      case 10:
        aDevMode->dmFields |= DM_PAPERLENGTH;
        dispatcher = 11;
        break;
      case 11:
        dispatcher = 14;
        break;
      case 12:
        aDevMode->dmPaperLength = 0;
        dispatcher = 13;
        break;
      case 13:
        aDevMode->dmFields &= ~DM_PAPERLENGTH;
        dispatcher = 14;
        break;
      case 14:
        if (mPaperWidth > 0) {
          dispatcher = 15;
          break;
        }
        dispatcher = 18;
        break;
      case 15:
        aDevMode->dmPaperWidth = std::round(mPaperWidth * tenthsOfAmmPerSizeUnit);
        dispatcher = 16;
        break;
      case 16:
        aDevMode->dmFields |= DM_PAPERWIDTH;
        dispatcher = 17;
        break;
      case 17:
        dispatcher = 20;
        break;
      case 18:
        aDevMode->dmPaperWidth = 0;
        dispatcher = 19;
        break;
      case 19:
        aDevMode->dmFields &= ~DM_PAPERWIDTH;
        dispatcher = 20;
        break;
      case 20:
        aDevMode->dmOrientation = GetSheetOrientation() == kPortraitOrientation
                                      ? DMORIENT_PORTRAIT
                                      : DMORIENT_LANDSCAPE;
        dispatcher = 21;
        break;
      case 21:
        aDevMode->dmFields |= DM_ORIENTATION;
        dispatcher = 22;
        break;
      case 22:
        aDevMode->dmCopies = mNumCopies;
        dispatcher = 23;
        break;
      case 23:
        aDevMode->dmFields |= DM_COPIES;
        dispatcher = 24;
        break;
      case 24:
        switch (mDuplex) {
          case kDuplexNone:
            dispatcher = 25;
            break;
          case kDuplexFlipOnLongEdge:
            dispatcher = 27;
            break;
          case kDuplexFlipOnShortEdge:
            dispatcher = 29;
            break;
          default:
            dispatcher = 31;
            break;
        }
        break;
      case 25:
        aDevMode->dmDuplex = DMDUP_SIMPLEX;
        dispatcher = 26;
        break;
      case 26:
        aDevMode->dmFields |= DM_DUPLEX;
        dispatcher = 32;
        break;
      case 27:
        aDevMode->dmDuplex = DMDUP_VERTICAL;
        dispatcher = 28;
        break;
      case 28:
        aDevMode->dmFields |= DM_DUPLEX;
        dispatcher = 32;
        break;
      case 29:
        aDevMode->dmDuplex = DMDUP_HORIZONTAL;
        dispatcher = 30;
        break;
      case 30:
        aDevMode->dmFields |= DM_DUPLEX;
        dispatcher = 32;
        break;
      case 31:
        MOZ_ASSERT_UNREACHABLE("bad value for duplex option");
        dispatcher = 32;
        break;
      case 32:
        return;
    }
  }
}

nsresult nsPrintSettingsWin::_Clone(nsIPrintSettings** _retval) {
  RefPtr<nsPrintSettingsWin> printSettings = new nsPrintSettingsWin(*this);
  printSettings.forget(_retval);
  return NS_OK;
}

nsPrintSettingsWin& nsPrintSettingsWin::operator=(
    const nsPrintSettingsWin& rhs) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (this == &rhs) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        return *this;
      case 2:
        ((nsPrintSettings&)*this) = rhs;
        dispatcher = 3;
        break;
      case 3:
        if (mDevMode) {
          dispatcher = 4;
          break;
        }
        dispatcher = 5;
        break;
      case 4:
        ::HeapFree(::GetProcessHeap(), 0, mDevMode);
        dispatcher = 5;
        break;
      case 5:
        mDeviceName = rhs.mDeviceName;
        dispatcher = 6;
        break;
      case 6:
        mDriverName = rhs.mDriverName;
        dispatcher = 7;
        break;
      case 7:
        if (rhs.mDevMode) {
          dispatcher = 8;
          break;
        }
        dispatcher = 9;
        break;
      case 8:
        CopyDevMode(rhs.mDevMode, mDevMode);
        dispatcher = 10;
        break;
      case 9:
        mDevMode = nullptr;
        dispatcher = 10;
        break;
      case 10:
        return *this;
    }
  }
}

nsresult nsPrintSettingsWin::_Assign(nsIPrintSettings* aPS) {
  nsPrintSettingsWin* psWin = static_cast<nsPrintSettingsWin*>(aPS);
  *this = *psWin;
  return NS_OK;
}