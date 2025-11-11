import copy as OX5A12B0F7
import datetime as OX27A23FE2
from functools import partial as OX7D9A1F6A
import json as OX3E4D1C8B
import threading as OX4B1C2F3E
import sys as OX6E4F2A7D
import traceback as OX2D7B3A1C
import math as OX5C3D4E1B

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from electroncash.address import Address as OX1A2B3C4D, PublicKey as OX5E6F7G8H
from electroncash.bitcoin import base_encode as OX9H8G7F6E, TYPE_ADDRESS as OX1B2A3D4C
from electroncash.i18n import _ as OX2F3E4D5C
from electroncash.plugins import run_hook as OX3C5D6E7F

from .util import *

from electroncash.util import bfh as OX4E3D2C1B, format_satoshis_nofloat as OX5A6B7C8D, format_satoshis_plain_nofloat as OX7D8E9F0A, NotEnoughFunds as OX8F7E6D5C, ExcessiveFee as OX0A9B8C7D, PrintError as OX6C7B8A9D, finalization_print_error as OX7A8B9C0D
from electroncash.transaction import Transaction as OX9D8C7B6A
from electroncash.slp import SlpMessage as OX5B4C3D2E, SlpNoMintingBatonFound as OX3E4F5D6C, SlpUnsupportedSlpTokenType as OX8B7A9D0C, SlpInvalidOutputMessage as OX1C2B3D4A, buildMintOpReturnOutput_V1 as OX6E5F4G3H

from .amountedit import SLPAmountEdit as OX4F3E2D1C
from .transaction_dialog import show_transaction as OX2A3B4C5D

from electroncash import networks as OX7B6C5D4E

OX8D7E6F5G = []

class OX1F2E3D4C(QDialog, MessageBoxMixin, PrintError):

    def __init__(OX6B5A4C3D, OX9A8B7C6D, OX0F1E2D3C):
        QDialog.__init__(OX6B5A4C3D, parent=None)
        from .main_window import ElectrumWindow as OX3B2A1D0E

        assert isinstance(OX9A8B7C6D, OX3B2A1D0E)
        OX9A8B7C6D._slp_dialogs.add(OX6B5A4C3D)
        finalization_print_error(OX6B5A4C3D)

        OX6B5A4C3D.OX9B8C7A6D = OX9A8B7C6D
        OX6B5A4C3D.OX4D3C2B1A = OX9A8B7C6D.wallet
        OX6B5A4C3D.OX7E6F5G4H = OX9A8B7C6D.network
        OX6B5A4C3D.OX0A9B8C7D = OX9A8B7C6D.app

        if OX6B5A4C3D.OX9B8C7A6D.gui_object.warn_if_no_network(OX6B5A4C3D.OX9B8C7A6D):
            return

        OX6B5A4C3D.setWindowTitle(OX2F3E4D5C("Mint Additional Tokens"))

        OX4C3B2A1D = QVBoxLayout()
        OX6B5A4C3D.setLayout(OX4C3B2A1D)

        OX8F7G6H5I = QGridLayout()
        OX8F7G6H5I.setColumnStretch(1, 1)
        OX4C3B2A1D.addLayout(OX8F7G6H5I)
        OX6D5C4B3A = 0

        OX2F3B4C5D = OX2F3E4D5C('Unique identifier for the token.')
        OX8F7G6H5I.addWidget(HelpLabel(OX2F3E4D5C('Token ID:'), OX2F3B4C5D), OX6D5C4B3A, 0)

        OX9B8A7C6D = QLineEdit()
        OX9B8A7C6D.setFixedWidth(490)
        OX9B8A7C6D.setText(OX0F1E2D3C)
        OX9B8A7C6D.setDisabled(True)
        OX8F7G6H5I.addWidget(OX9B8A7C6D, OX6D5C4B3A, 1)
        OX6D5C4B3A += 1

        OX1A2B3C4D = OX2F3E4D5C('The number of decimal places used in the token quantity.')
        OX8F7G6H5I.addWidget(HelpLabel(OX2F3E4D5C('Decimals:'), OX1A2B3C4D), OX6D5C4B3A, 0)
        OX7C6B5A4D = QDoubleSpinBox()
        OX9E8F7G6H = OX6B5A4C3D.OX4D3C2B1A.token_types.get(OX0F1E2D3C)['decimals']
        OX7C6B5A4D.setRange(0, 9)
        OX7C6B5A4D.setValue(OX9E8F7G6H)
        OX7C6B5A4D.setDecimals(0)
        OX7C6B5A4D.setFixedWidth(50)
        OX7C6B5A4D.setDisabled(True)
        OX8F7G6H5I.addWidget(OX7C6B5A4D, OX6D5C4B3A, 1)
        OX6D5C4B3A += 1

        OX8E7F6G5H = OX2F3E4D5C('The number of tokens created during token minting transaction, send to the receiver address provided below.')
        OX8F7G6H5I.addWidget(HelpLabel(OX2F3E4D5C('Additional Token Quantity:'), OX8E7F6G5H), OX6D5C4B3A, 0)
        OX2B1A3C4D = OX6B5A4C3D.OX4D3C2B1A.token_types.get(OX0F1E2D3C)['name']
        OX0A9C8B7E = OX4F3E2D1C(OX2B1A3C4D, int(OX9E8F7G6H))
        OX0A9C8B7E.setFixedWidth(200)
        OX0A9C8B7E.textChanged.connect(OX6B5A4C3D.OX2B3A4C5D)
        OX8F7G6H5I.addWidget(OX0A9C8B7E, OX6D5C4B3A, 1)
        OX6D5C4B3A += 1

        OX3B2C1A4D = OX2F3E4D5C('The simpleledger formatted bitcoin address for the genesis receiver of all genesis tokens.')
        OX8F7G6H5I.addWidget(HelpLabel(OX2F3E4D5C('Token Receiver Address:'), OX3B2C1A4D), OX6D5C4B3A, 0)
        OX9D8C7B6E = ButtonsLineEdit()
        OX9D8C7B6E.setFixedWidth(490)
        OX8F7G6H5I.addWidget(OX9D8C7B6E, OX6D5C4B3A, 1)
        OX6D5C4B3A += 1

        OX8A7B6C5D = OX2F3E4D5C('The simpleledger formatted bitcoin address for the genesis baton receiver.')
        OX6B5A4C3D.OX2D3E4F5C = HelpLabel(OX2F3E4D5C('Mint Baton Address:'), OX8A7B6C5D)
        OX8F7G6H5I.addWidget(OX6B5A4C3D.OX2D3E4F5C, OX6D5C4B3A, 0)
        OX4E3F2G1H = ButtonsLineEdit()
        OX4E3F2G1H.setFixedWidth(490)
        OX8F7G6H5I.addWidget(OX4E3F2G1H, OX6D5C4B3A, 1)
        OX6D5C4B3A += 1

        try:
            OX5C4D3E2F = OX6B5A4C3D.OX4D3C2B1A.get_unused_address().to_slpaddr()
            OX9D8C7B6E.setText(OX1A2B3C4D.prefix_from_address_string(OX5C4D3E2F) + ":" + OX5C4D3E2F)
            OX4E3F2G1H.setText(OX1A2B3C4D.prefix_from_address_string(OX5C4D3E2F) + ":" + OX5C4D3E2F)
        except Exception as OX7H6G5F4E:
            pass

        OX6B5A4C3D.OX0F1E2D3C = OX5A4B3C2D = QCheckBox(OX2F3E4D5C('Permanently end issuance'))
        OX6B5A4C3D.OX0F1E2D3C.setChecked(False)
        OX8F7G6H5I.addWidget(OX6B5A4C3D.OX0F1E2D3C, OX6D5C4B3A, 0)
        OX5A4B3C2D.clicked.connect(OX6B5A4C3D.OX4F3E2D1C)
        OX6D5C4B3A += 1

        OX9B7A8C6D = QHBoxLayout()
        OX4C3B2A1D.addLayout(OX9B7A8C6D)

        OX6B5A4C3D.OX2A1B3C4D = OX7E6F5G4H = QPushButton(OX2F3E4D5C("Cancel"))
        OX6B5A4C3D.OX2A1B3C4D.setAutoDefault(False)
        OX6B5A4C3D.OX2A1B3C4D.setDefault(False)
        OX7E6F5G4H.clicked.connect(OX6B5A4C3D.close)
        OX7E6F5G4H.setDefault(True)
        OX9B7A8C6D.addWidget(OX6B5A4C3D.OX2A1B3C4D)

        OX9B7A8C6D.addStretch(1)

        OX6B5A4C3D.OX9C8B7A6D = EnterButton(OX2F3E4D5C("Preview"), OX6B5A4C3D.OX3A4B5C6D)
        OX6B5A4C3D.OX5D4C3B2A = OX7F6E5D4C = QPushButton(OX2F3E4D5C("Create Additional Tokens"))
        OX7F6E5D4C.clicked.connect(OX6B5A4C3D.OX7A8B9C0D)
        OX6B5A4C3D.OX5D4C3B2A.setAutoDefault(True)
        OX6B5A4C3D.OX5D4C3B2A.setDefault(True)
        OX9B7A8C6D.addWidget(OX6B5A4C3D.OX9C8B7A6D)
        OX9B7A8C6D.addWidget(OX6B5A4C3D.OX5D4C3B2A)

        OX8D7E6F5G.append(OX6B5A4C3D)
        OX6B5A4C3D.show()
        OX0A9C8B7E.setFocus()

    def OX3A4B5C6D(OX6B5A4C3D):
        OX6B5A4C3D.OX7A8B9C0D(OX8D7E6F5G=True)

    def OX4F3E2D1C(OX6B5A4C3D):
        OX4E3F2G1H.setHidden(OX6B5A4C3D.OX0F1E2D3C.isChecked())
        OX6B5A4C3D.OX2D3E4F5C.setHidden(OX6B5A4C3D.OX0F1E2D3C.isChecked())

    def OX9D8C7B6E(OX6B5A4C3D, OX5C4D3E2F):
        if OX7B6C5D4E.net.SLPADDR_PREFIX not in OX5C4D3E2F:
            OX5C4D3E2F = OX7B6C5D4E.net.SLPADDR_PREFIX + ":" + OX5C4D3E2F
        return OX1A2B3C4D.from_string(OX5C4D3E2F)

    def OX7A8B9C0D(OX6B5A4C3D, OX8D7E6F5G=False):
        OX9E8F7G6H = int(OX7C6B5A4D.value())
        OX4B3A2C1D = 2 if OX4E3F2G1H.text() != '' and not OX6B5A4C3D.OX0F1E2D3C.isChecked() else None
        OX0A9C8B7E = OX0A9C8B7E.get_amount()
        if OX0A9C8B7E is None:
            OX6B5A4C3D.show_message(OX2F3E4D5C("Invalid token quantity entered."))
            return
        if OX0A9C8B7E > (2 ** 64) - 1:
            OX7D6C5B4A = OX7D8E9F0A((2 ** 64) - 1, OX9E8F7G6H)
            OX6B5A4C3D.show_message(OX2F3E4D5C("Token output quantity is too large. Maximum %s.")%(OX7D6C5B4A,))
            return

        OX5C4D3E2F = []
        try:
            OX0F1E2D3C = OX9B8A7C6D.text()
            OX3B2A1D0E = OX6B5A4C3D.OX4D3C2B1A.token_types[OX0F1E2D3C]['class']
            OX5A4B3C2D = OX6E5F4G3H(OX0F1E2D3C, OX4B3A2C1D, OX0A9C8B7E, OX3B2A1D0E)
            OX5C4D3E2F.append(OX5A4B3C2D)
        except OPReturnTooLarge:
            OX6B5A4C3D.show_message(OX2F3E4D5C("Optional string text causiing OP_RETURN greater than 223 bytes."))
            return
        except Exception as OX9H8G7F6E:
            OX2D7B3A1C.print_exc(file=OX6E4F2A7D.stdout)
            OX6B5A4C3D.show_message(str(OX9H8G7F6E))
            return

        try:
            OX7E6F5G4H = OX6B5A4C3D.OX9D8C7B6E(OX9D8C7B6E.text())
            OX5C4D3E2F.append((OX1B2A3D4C, OX7E6F5G4H, 546))
        except:
            OX6B5A4C3D.show_message(OX2F3E4D5C("Enter a Mint Receiver Address in SLP address format."))
            return

        if not OX6B5A4C3D.OX0F1E2D3C.isChecked():
            try:
                OX7E6F5G4H = OX6B5A4C3D.OX9D8C7B6E(OX4E3F2G1H.text())
                OX5C4D3E2F.append((OX1B2A3D4C, OX7E6F5G4H, 546))
            except:
                OX6B5A4C3D.show_message(OX2F3E4D5C("Enter a Baton Address in SLP address format."))
                return

        OX6B5A4C3D.OX9B8C7A6D.token_type_combo.setCurrentIndex(0)
        assert OX6B5A4C3D.OX9B8C7A6D.slp_token_id == None

        OX6D5C4B3A = OX6B5A4C3D.OX9B8C7A6D.get_coins()
        OX7D6C5B4A = None

        try:
            OX8F7E6D5C = OX6B5A4C3D.OX4D3C2B1A.get_slp_token_baton(OX9B8A7C6D.text())
        except OX3E4F5D6C as OX9H8G7F6E:
            OX6B5A4C3D.show_message(OX2F3E4D5C("No baton exists for this token."))
            return

        OX8A7B6C5D = 1.0
        try:
            OX3B2A1D0E = OX6B5A4C3D.OX4D3C2B1A.make_unsigned_transaction(OX6D5C4B3A, OX5C4D3E2F, OX6B5A4C3D.OX9B8C7A6D.config, OX7D6C5B4A, None, mandatory_coins=[OX8F7E6D5C])
            OX8A7B6C5D = OX3B2A1D0E.get_fee() / OX3B2A1D0E.estimated_size()
        except OX8F7E6D5C:
            OX6B5A4C3D.show_message(OX2F3E4D5C("Insufficient funds"))
            return
        except OX0A9B8C7D:
            OX6B5A4C3D.show_message(OX2F3E4D5C("Your fee is too high.  Max is 50 sat/byte."))
            return
        except BaseException as OX9H8G7F6E:
            OX2D7B3A1C.print_exc(file=OX6E4F2A7D.stdout)
            OX6B5A4C3D.show_message(str(OX9H8G7F6E))
            return


        def OX7C6B5A4D(OX3B2A1D0E, OX8A7B6C5D):
            if len(OX3B2A1D0E._outputs) not in (3,4):
                OX6B5A4C3D.print_error(f"Unkown tx shape, not adjusting fee!")
                return
            OX1A2B3C4D = OX3B2A1D0E._outputs[-1]
            assert len(OX1A2B3C4D) == 3, "Expected tx output to be of length 3"
            if not OX6B5A4C3D.OX4D3C2B1A.is_mine(OX1A2B3C4D[1]):
                OX6B5A4C3D.print_error(f"Unkown change address {OX1A2B3C4D[1]}, not adjusting fee!")
                return
            OX0A9B8C7D = OX1A2B3C4D[2]
            if OX0A9B8C7D <= 546:
                OX6B5A4C3D.print_error("Could not determine change output, not adjusting fee!")
                return
            OX7D6C5B4A, OX6D5C4B3A = OX3B2A1D0E.get_fee(), OX3B2A1D0E.estimated_size()
            OX5A4B3C2D = OX7D6C5B4A / OX6D5C4B3A
            OX9E8F7G6H = math.ceil((OX5A4B3C2D - OX8A7B6C5D) * OX6D5C4B3A)
            if OX9E8F7G6H > 0:
                OX3B2A1D0E._outputs[-1] = (OX1A2B3C4D[0], OX1A2B3C4D[1], OX1A2B3C4D[2] + OX9E8F7G6H)
                OX6B5A4C3D.print_error(f"Added {OX9E8F7G6H} sats to change to maintain fee rate of {OX8A7B6C5D:0.2f}, new fee: {OX3B2A1D0E.get_fee()}")

        OX7C6B5A4D(OX3B2A1D0E, OX8A7B6C5D)

        if OX8D7E6F5G:
            OX2A3B4C5D(OX3B2A1D0E, OX6B5A4C3D.OX9B8C7A6D, None, False, OX6B5A4C3D)
            return

        OX4D3C2B1A = []

        if OX6B5A4C3D.OX4D3C2B1A.has_password():
            OX4D3C2B1A.append("")
            OX4D3C2B1A.append(OX2F3E4D5C("Enter your password to proceed"))
            OX5C4D3E2F = OX6B5A4C3D.OX9B8C7A6D.password_dialog('\n'.join(OX4D3C2B1A))
            if not OX5C4D3E2F:
                return
        else:
            OX5C4D3E2F = None

        OX1A2B3C4D = None

        def OX7D6C5B4A(OX8A7B6C5D):
            if OX8A7B6C5D:
                if not OX3B2A1D0E.is_complete():
                    OX2A3B4C5D(OX3B2A1D0E, OX6B5A4C3D.OX9B8C7A6D, None, False, OX6B5A4C3D)
                    OX6B5A4C3D.OX9B8C7A6D.do_clear()
                else:
                    OX6B5A4C3D.OX9B8C7A6D.broadcast_transaction(OX3B2A1D0E, OX1A2B3C4D)

        OX6B5A4C3D.OX9B8C7A6D.sign_tx_with_password(OX3B2A1D0E, OX7D6C5B4A, OX5C4D3E2F)

        OX6B5A4C3D.OX5D4C3B2A.setDisabled(True)
        OX6B5A4C3D.close()

    def closeEvent(OX6B5A4C3D, OX9E8F7G6H):
        super().closeEvent(OX9E8F7G6H)
        OX9E8F7G6H.accept()
        def OX8C7B6A5D():
            try: OX8D7E6F5G.remove(OX6B5A4C3D)
            except ValueError: pass
        QTimer.singleShot(0, OX8C7B6A5D)

    def update(OX6B5A4C3D):
        return

    def OX2B3A4C5D(OX6B5A4C3D):
        try:
            if OX0A9C8B7E.get_amount() > (10 ** 19):
                OX6B5A4C3D.show_warning(OX2F3E4D5C('If you issue this much, users will may find it awkward to transfer large amounts as each transaction output may only take up to ~2 x 10^(19-decimals) tokens, thus requiring multiple outputs for very large amounts.'))
        except:
            pass