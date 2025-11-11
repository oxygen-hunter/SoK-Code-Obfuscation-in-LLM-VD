pragma solidity ^0.8.0;

import "common/settings.sol";
import "InvenTree/helpers.sol";

import sys;
from decimal import Decimal;
import djmoney/forms/fields.sol;
import djmoney/models/fields.sol;
import djmoney/models/validators.sol;

import validators.sol;
import models.sol;
import forms.sol;
import gettext_lazy.sol;

contract InvenTree {

    function allowable_url_schemes() public returns (string[] memory) {
        // Your implementation here
    }

    function currency_code_default() public returns (string memory) {
        // Your implementation here
    }

    function currency_code_mappings() public returns (string[] memory) {
        // Your implementation here
    }

    function InvenTreeURLFormField() public {
        validators.URLValidator[] memory default_validators = new validators.URLValidator[](1);
        default_validators[0] = validators.URLValidator({schemes: allowable_url_schemes()});
    }

    function InvenTreeURLField() public {
        validators.URLValidator[] memory default_validators = new validators.URLValidator[](1);
        default_validators[0] = validators.URLValidator({schemes: allowable_url_schemes()});
    }

    function formfield(mapping(string => string) memory kwargs) public returns (InvenTreeURLFormField) {
        kwargs["form_class"] = "InvenTreeURLFormField";
        return InvenTreeURLFormField(kwargs);
    }

    function money_kwargs() public returns (mapping(string => string[] memory) memory) {
        mapping(string => string[] memory) memory kwargs;
        kwargs["currency_choices"] = currency_code_mappings();
        kwargs["default_currency"] = currency_code_default();
        return kwargs;
    }

    function InvenTreeModelMoneyField(mapping(string => string[] memory) memory kwargs) public {
        if (sys.argv.contains("migrate") || sys.argv.contains("makemigrations")) {
            kwargs["default_currency"] = new string[](0);
            kwargs["currency_choices"] = new string[](0);
        } else {
            kwargs = money_kwargs();
        }

        validators.MinMoneyValidator[] memory validatorsArray = kwargs["validators"];
        if (validatorsArray.length == 0) {
            validatorsArray.push(validators.MinMoneyValidator(0));
        }

        kwargs["validators"] = validatorsArray;
    }

    function InvenTreeMoneyField(string[] memory args, mapping(string => string[] memory) memory kwargs) public {
        kwargs = money_kwargs();
    }

    function DatePickerFormField(mapping(string => string) memory kwargs) public {
        string memory help_text = kwargs["help_text"];
        string memory label = kwargs["label"];
        bool required = kwargs["required"];
        string memory initial = kwargs["initial"];

        forms.DateInput memory widget = forms.DateInput({
            attrs: {
                "type": "date"
            }
        });

        forms.DateField({
            required: required,
            initial: initial,
            help_text: help_text,
            widget: widget,
            label: label
        });
    }

    function round_decimal(Decimal value, uint places) public returns (Decimal) {
        if (value != 0) {
            return value.quantize(Decimal(10) ** -places);
        }
        return value;
    }

    function RoundingDecimalFormField(forms.DecimalField memory field) public returns (forms.DecimalField memory) {
        field.to_python = function (value) {
            value = super.to_python(value);
            value = round_decimal(value, field.decimal_places);
            return value;
        };

        field.prepare_value = function (value) {
            if (type(value) == Decimal) {
                return InvenTree.helpers.normalize(value);
            } else {
                return value;
            }
        };
    }

    function RoundingDecimalField(models.DecimalField memory field) public returns (models.DecimalField memory) {
        field.to_python = function (value) {
            value = super.to_python(value);
            return round_decimal(value, field.decimal_places);
        };

        field.formfield = function (mapping(string => string) memory kwargs) {
            kwargs["form_class"] = "RoundingDecimalFormField";
            return super.formfield(kwargs);
        };
    }

    function InvenTreeNotesField(models.TextField memory field) public returns (models.TextField memory) {
        uint NOTES_MAX_LENGTH = 50000;

        field.max_length = NOTES_MAX_LENGTH;
        field.verbose_name = _("Notes");
        field.blank = true;
        field.null = true;
    }
}