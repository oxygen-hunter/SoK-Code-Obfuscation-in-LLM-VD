extern "C" {
#include <Python.h>
}

#include <QDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include <QThread>
#include <QVariant>

extern "C" {
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <stdarg.h>
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <json/json.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <exception>
#include <typeinfo>
#include <locale>
#include <codecvt>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "electroncash.h"
#include "util.h"
#include "amountedit.h"
#include "transaction_dialog.h"

using namespace std;

extern "C" {
#include <slp.h>
#include <slp_transaction.h>
#include <slp_mint.h>
}

std::vector<QDialog*> dialogs;  

class SlpCreateTokenMintDialog : public QDialog, public MessageBoxMixin, public PrintError {
public:
    SlpCreateTokenMintDialog(ElectrumWindow* main_window, const std::string& token_id_hex) {
        // We want to be a top-level window
        this->setParent(nullptr);
        assert(dynamic_cast<ElectrumWindow*>(main_window));
        main_window->_slp_dialogs.add(this);
        finalization_print_error(this);

        this->main_window = main_window;
        this->wallet = main_window->wallet;
        this->network = main_window->network;
        this->app = main_window->app;

        if (main_window->gui_object.warn_if_no_network(main_window)) {
            return;
        }

        this->setWindowTitle(QString::fromStdString(_("Mint Additional Tokens")));

        QVBoxLayout* vbox = new QVBoxLayout();
        this->setLayout(vbox);

        QGridLayout* grid = new QGridLayout();
        grid->setColumnStretch(1, 1);
        vbox->addLayout(grid);
        int row = 0;

        QString msg = QString::fromStdString(_('Unique identifier for the token.'));
        grid->addWidget(new HelpLabel(QString::fromStdString(_('Token ID:')), msg), row, 0);

        QLineEdit* token_id_e = new QLineEdit();
        token_id_e->setFixedWidth(490);
        token_id_e->setText(QString::fromStdString(token_id_hex));
        token_id_e->setDisabled(true);
        grid->addWidget(token_id_e, row, 1);
        row += 1;

        msg = QString::fromStdString(_('The number of decimal places used in the token quantity.'));
        grid->addWidget(new HelpLabel(QString::fromStdString(_('Decimals:')), msg), row, 0);
        QDoubleSpinBox* token_dec = new QDoubleSpinBox();
        int decimals = main_window->wallet.token_types[token_id_hex]["decimals"].asInt();
        token_dec->setRange(0, 9);
        token_dec->setValue(decimals);
        token_dec->setDecimals(0);
        token_dec->setFixedWidth(50);
        token_dec->setDisabled(true);
        grid->addWidget(token_dec, row, 1);
        row += 1;

        msg = QString::fromStdString(_('The number of tokens created during token minting transaction, send to the receiver address provided below.'));
        grid->addWidget(new HelpLabel(QString::fromStdString(_('Additional Token Quantity:')), msg), row, 0);
        std::string name = main_window->wallet.token_types[token_id_hex]["name"].asString();
        SLPAmountEdit* token_qty_e = new SLPAmountEdit(name, decimals);
        token_qty_e->setFixedWidth(200);
        QObject::connect(token_qty_e, &SLPAmountEdit::textChanged, [this] { this->check_token_qty(); });
        grid->addWidget(token_qty_e, row, 1);
        row += 1;

        msg = QString::fromStdString(_('The simpleledger formatted bitcoin address for the genesis receiver of all genesis tokens.'));
        grid->addWidget(new HelpLabel(QString::fromStdString(_('Token Receiver Address:')), msg), row, 0);
        ButtonsLineEdit* token_pay_to_e = new ButtonsLineEdit();
        token_pay_to_e->setFixedWidth(490);
        grid->addWidget(token_pay_to_e, row, 1);
        row += 1;

        msg = QString::fromStdString(_('The simpleledger formatted bitcoin address for the genesis baton receiver.'));
        QLabel* token_baton_label = new HelpLabel(QString::fromStdString(_('Mint Baton Address:')), msg);
        grid->addWidget(token_baton_label, row, 0);
        ButtonsLineEdit* token_baton_to_e = new ButtonsLineEdit();
        token_baton_to_e->setFixedWidth(490);
        grid->addWidget(token_baton_to_e, row, 1);
        row += 1;

        try {
            std::string slpAddr = this->wallet->get_unused_address().to_slpaddr();
            token_pay_to_e->setText(QString::fromStdString(Address::prefix_from_address_string(slpAddr) + ":" + slpAddr));
            token_baton_to_e->setText(QString::fromStdString(Address::prefix_from_address_string(slpAddr) + ":" + slpAddr));
        } catch (const std::exception& e) {
            // handle exception
        }

        QCheckBox* token_fixed_supply_cb = new QCheckBox(QString::fromStdString(_('Permanently end issuance')));
        token_fixed_supply_cb->setChecked(false);
        grid->addWidget(token_fixed_supply_cb, row, 0);
        QObject::connect(token_fixed_supply_cb, &QCheckBox::clicked, [this] { this->show_mint_baton_address(); });
        row += 1;

        QHBoxLayout* hbox = new QHBoxLayout();
        vbox->addLayout(hbox);

        QPushButton* cancel_button = new QPushButton(QString::fromStdString(_("Cancel")));
        cancel_button->setAutoDefault(false);
        cancel_button->setDefault(false);
        QObject::connect(cancel_button, &QPushButton::clicked, [this] { this->close(); });
        cancel_button->setDefault(true);
        hbox->addWidget(cancel_button);

        hbox->addStretch(1);

        EnterButton* preview_button = new EnterButton(QString::fromStdString(_("Preview")), [this] { this->do_preview(); });
        QPushButton* mint_button = new QPushButton(QString::fromStdString(_("Create Additional Tokens")));
        QObject::connect(mint_button, &QPushButton::clicked, [this] { this->mint_token(); });
        mint_button->setAutoDefault(true);
        mint_button->setDefault(true);
        hbox->addWidget(preview_button);
        hbox->addWidget(mint_button);

        dialogs.push_back(this);
        this->show();
        token_qty_e->setFocus();
    }

    void do_preview() {
        this->mint_token(true);
    }

    void show_mint_baton_address() {
        token_baton_to_e->setHidden(token_fixed_supply_cb->isChecked());
        token_baton_label->setHidden(token_fixed_supply_cb->isChecked());
    }

    Address parse_address(const std::string& address) {
        if (address.find(networks.net.SLPADDR_PREFIX) == std::string::npos) {
            address = networks.net.SLPADDR_PREFIX + ":" + address;
        }
        return Address::from_string(address);
    }

    void mint_token(bool preview = false) {
        int decimals = static_cast<int>(token_dec->value());
        int mint_baton_vout = (token_baton_to_e->text() != "" && !token_fixed_supply_cb->isChecked()) ? 2 : -1;
        int init_mint_qty = token_qty_e->get_amount();
        if (init_mint_qty == -1) {
            this->show_message(QString::fromStdString(_("Invalid token quantity entered.")));
            return;
        }
        if (init_mint_qty > (2LL << 64) - 1) {
            std::string maxqty = format_satoshis_plain_nofloat((2LL << 64) - 1, decimals);
            this->show_message(QString::fromStdString(_("Token output quantity is too large. Maximum ") + maxqty + "."));
            return;
        }

        std::vector<std::tuple<int, Address, int>> outputs;
        try {
            std::string token_id_hex = token_id_e->text().toStdString();
            int token_type = wallet.token_types[token_id_hex]["class"].asInt();
            auto slp_op_return_msg = buildMintOpReturnOutput_V1(token_id_hex, mint_baton_vout, init_mint_qty, token_type);
            outputs.push_back(slp_op_return_msg);
        } catch (const OPReturnTooLarge&) {
            this->show_message(QString::fromStdString(_("Optional string text causing OP_RETURN greater than 223 bytes.")));
            return;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            this->show_message(QString::fromStdString(e.what()));
            return;
        }

        try {
            Address addr = parse_address(token_pay_to_e->text().toStdString());
            outputs.push_back(std::make_tuple(TYPE_ADDRESS, addr, 546));
        } catch (const std::exception&) {
            this->show_message(QString::fromStdString(_("Enter a Mint Receiver Address in SLP address format.")));
            return;
        }

        if (!token_fixed_supply_cb->isChecked()) {
            try {
                Address addr = parse_address(token_baton_to_e->text().toStdString());
                outputs.push_back(std::make_tuple(TYPE_ADDRESS, addr, 546));
            } catch (const std::exception&) {
                this->show_message(QString::fromStdString(_("Enter a Baton Address in SLP address format.")));
                return;
            }
        }

        main_window->token_type_combo.setCurrentIndex(0);
        assert(main_window->slp_token_id == nullptr);

        std::vector<Coin> coins = main_window->get_coins();
        double fee = 0.0;

        try {
            auto baton_input = main_window->wallet.get_slp_token_baton(token_id_e->text().toStdString());
        } catch (const SlpNoMintingBatonFound& e) {
            this->show_message(QString::fromStdString(_("No baton exists for this token.")));
            return;
        }

        double desired_fee_rate = 1.0;
        try {
            auto tx = main_window->wallet.make_unsigned_transaction(coins, outputs, main_window->config, fee, nullptr, std::vector<Coin>{baton_input});
            desired_fee_rate = tx.get_fee() / tx.estimated_size();
        } catch (const NotEnoughFunds&) {
            this->show_message(QString::fromStdString(_("Insufficient funds")));
            return;
        } catch (const ExcessiveFee&) {
            this->show_message(QString::fromStdString(_("Your fee is too high. Max is 50 sat/byte.")));
            return;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            this->show_message(QString::fromStdString(e.what()));
            return;
        }

        tx_adjust_change_amount_based_on_baton_amount(tx, desired_fee_rate);

        if (preview) {
            show_transaction(tx, main_window, nullptr, false, this);
            return;
        }

        std::vector<std::string> msg;

        if (main_window->wallet.has_password()) {
            msg.push_back("");
            msg.push_back(_("Enter your password to proceed"));
            std::string password = main_window->password_dialog(msg.join('\n'));
            if (password.empty()) {
                return;
            }
        } else {
            std::string password;
        }

        std::string tx_desc;

        auto sign_done = [this](bool success) {
            if (success) {
                if (!tx.is_complete()) {
                    show_transaction(tx, main_window, nullptr, false, this);
                    main_window->do_clear();
                } else {
                    main_window->broadcast_transaction(tx, tx_desc);
                }
            }
        };

        main_window->sign_tx_with_password(tx, sign_done, password);

        mint_button->setDisabled(true);
        this->close();
    }

    virtual void closeEvent(QCloseEvent* event) override {
        QDialog::closeEvent(event);
        event->accept();
        auto remove_self = [this] {
            auto it = std::remove(dialogs.begin(), dialogs.end(), this);
            if (it != dialogs.end()) {
                dialogs.erase(it);
            }
        };
        QTimer::singleShot(0, remove_self);
    }

    void update() {}

    void check_token_qty() {
        try {
            if (token_qty_e->get_amount() > (10LL << 19)) {
                this->show_warning(QString::fromStdString(_('If you issue this much, users will may find it awkward to transfer large amounts as each transaction output may only take up to ~2 x 10^(19-decimals) tokens, thus requiring multiple outputs for very large amounts.')));
            }
        } catch (const std::exception&) {
            // handle exception
        }
    }
};