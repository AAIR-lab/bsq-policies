#include "action.h"

bool Action::operator== (const Action& ref) const {
    if (action_name != ref.action_name) {
        return false;
    }
    if (string_params != ref.string_params) {
        return false;
    }
    if (integer_params != ref.integer_params) {
        return false;
    }
    if (bool_params != ref.bool_params) {
        return false;
    }
    if (double_params != ref.double_params) {
        return false;
    }
    return true;
}