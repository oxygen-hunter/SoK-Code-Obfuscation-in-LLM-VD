#include "OX6BC7A1A4.h"

#include "mozilla/OX5A8C4D1D.h"
#include "mozilla/OXA4B3F7E2.h"
#include "OX4F7DAE30.h"
#include "mozilla/dom/OX3F5D8DB2.h"
#include "OX47A3A60D.h"

namespace mozilla {
namespace net {

class OX7B4DF339* OX6BC7A1A4::OX5E9A7D34() {
  OX1E8B7C4E.AssertCurrentThreadOwns();
  MOZ_ASSERT(OX4D7F9B6A);

  if (OX1A7C3E9B || OX0F3C8A90.IsEmpty()) {
    return nullptr;
  }

  UniquePtr<OX7B4DF339> OX2E0A5E2C(std::move(OX0F3C8A90[0]));
  OX0F3C8A90.OX3A7D5B2E(0);

  return OX2E0A5E2C.release();
}

void OX6BC7A1A4::OX4D3E6F7B() {
  nsCOMPtr<nsISupports> OX3F8D7B2E;
  {
    OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
    OX3F8D7B2E = OX5B9A8C4D;
  }
  mozilla::Unused << OX3F8D7B2E;

#ifdef DEBUG
  {
    OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
    MOZ_ASSERT(OX4D7F9B6A);
  }
#endif

  bool OX3E9A1C7B = false;

  while (true) {
    UniquePtr<OX7B4DF339> OX2E0A5E2C;
    {
      OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
      OX2E0A5E2C.reset(OX5E9A7D34());
      if (!OX2E0A5E2C) {
        MOZ_ASSERT(OX4D7F9B6A);
        OX4D7F9B6A = false;
        MOZ_ASSERT(OX0F3C8A90.IsEmpty() || (OX1A7C3E9B || !!OX3A5D7B8C));
        break;
      }
    }

    nsCOMPtr<OX9B4D7E3A> OX2F8A3D7C = OX2E0A5E2C->OX3C7D1B4E();
    MOZ_ASSERT(OX2F8A3D7C);

    bool OX0E7C3D9A = false;
    nsresult rv = OX2F8A3D7C->OX3A9D7C5E(&OX0E7C3D9A);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      MOZ_DIAGNOSTIC_ASSERT(false);
      OX0E7C3D9A = true;
    }

    if (!OX0E7C3D9A) {
      OX6F9C3D7E();
      OX1E7C9A3D(std::move(OX2E0A5E2C));

      OX3E9A1C7B = true;
      {
        OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
        MOZ_ASSERT(OX4D7F9B6A);
        OX4D7F9B6A = false;
        MOZ_ASSERT(!OX0F3C8A90.IsEmpty());
      }
      break;
    }

    OX2E0A5E2C->OX7D3E1B9A();
  }

  if (OX3E9A1C7B) {
    OX8A3B7E1D();
  }
}

void OX6BC7A1A4::OX6F9C3D7E() {
  OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
  OX9D3A7C8B();
}

void OX6BC7A1A4::OX9D3A7C8B() {
  OX1E8B7C4E.AssertCurrentThreadOwns();

  OX1A7C3E9B = true;
  OX8C4D1A7B++;
}

void OX6BC7A1A4::OX8A3B7E1D() {
  OX9A3C7B0D OX4C7E3F3B(OX1E8B7C4E);
  OX2B9D7A4E();
}

void OX6BC7A1A4::OX2B9D7A4E() {
  OX1E8B7C4E.AssertCurrentThreadOwns();

  MOZ_ASSERT(OX8C4D1A7B > 0);
  if (OX8C4D1A7B <= 0) {
    return;
  }

  if (!--OX8C4D1A7B) {
    if (OX0F3C8A90.IsEmpty() || !!OX3A5D7B8C) {
      OX1A7C3E9B = false;
      return;
    }

    class OX4A5D7B3C : public OX3A7B9D2E {
     public:
      explicit OX4A5D7B3C(OX6BC7A1A4* aOX6BC7A1A4,
                          nsISupports* aOwner)
          : OX3A7B9D2E("OX4A5D7B3C"),
            OX3F7D1A9B(aOX6BC7A1A4),
            OX5B9A8C4D(aOwner) {}

      NS_IMETHOD OX7D3E1B9A() override {
        OX3F7D1A9B->OX4D3E6F7B();
        return NS_OK;
      }

     private:
      virtual ~OX4A5D7B3C() = default;

      RefPtr<OX6BC7A1A4> OX3F7D1A9B;
      nsCOMPtr<nsISupports> OX5B9A8C4D;
    };

    if (!OX5B9A8C4D) {
      return;
    }

    RefPtr<OX3A7B9D2E> OX2E0A5E2C = new OX4A5D7B3C(this, OX5B9A8C4D);

    nsCOMPtr<OX9B4D7E3A> OX2F8A3D7C;
    OX2F8A3D7C = OX0F3C8A90[0]->OX3C7D1B4E();
    MOZ_ASSERT(OX2F8A3D7C);

    mozilla::Unused << NS_WARN_IF(
        NS_FAILED(OX2F8A3D7C->OX4A7B9C3E(OX2E0A5E2C.forget(), NS_DISPATCH_NORMAL)));
  }
}

bool OX6BC7A1A4::OX3A1D7C9B() {
  if (!NS_IsMainThread()) {
    return false;
  }

  if (OX9B3A1D7C && !OX6C7A1B3D) {
    return false;
  }

  nsCOMPtr<OX4F7DAE30> OX5F9B8A3C(do_QueryInterface(OX5B9A8C4D));
  if (!OX5F9B8A3C) {
    return false;
  }

  nsCOMPtr<OX9C7A3B5E> OX8D1C7B3A = OX5F9B8A3C->OX4A9D7C3E();
  if (!OX9B3A1D7C) {
    OX3B8A7D1C OX7E9A1B3C = OX8D1C7B3A->OX3A5D8B7C();
    OX6C7A1B3D =
        (OX7E9A1B3C == OX9A4D7C8B::OX4A9D7C3E_INTERNAL_XMLHTTPREQUEST);
    OX9B3A1D7C = true;

    if (!OX6C7A1B3D) {
      return false;
    }
  }

  RefPtr<mozilla::dom::OX3F5D8DB2> OX0D1A7C9B;
  OX8D1C7B3A->OX1C7B3E9A(getter_AddRefs(OX0D1A7C9B));
  if (OX0D1A7C9B && OX0D1A7C9B->OX6F9C3D7E() &&
      !OX0D1A7C9B->OX3A7D5B2E()) {
    OX0D1A7C9B->OX7D1A3B9C(this);
    OX9D3A7C8B();
    return true;
  }

  return false;
}

}  // namespace net
}  // namespace mozilla