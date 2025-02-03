#include <stdexcept>
#include "query_instance.h"
#include "utility.h"
#include <iostream>

QueryInstance::~QueryInstance()
{
}

BoolQueryInstance::~BoolQueryInstance()
{
}

BoolQueryInstance::BoolQueryInstance(unsigned int x)
{
    target_index = x;
}

QueryInstance* BoolQueryInstance::create_copy() const
{
    return new BoolQueryInstance(*this);
}

bool BoolQueryInstance::evaluate(const StaticState &static_ref) const
{
    if (dynamic) {
        throw std::invalid_argument("Cannot evaluate a dynamic function with static portion of belief state.");
    }
    if (negated) {
        return !static_ref.bool_params[target_index];
    }
    return static_ref.bool_params[target_index];
}

bool BoolQueryInstance::evaluate(const StaticState &static_ref, const DynamicState &dynamic_ref) const
{
    if (dynamic) {
        if (negated) {
            return !dynamic_ref.bool_params[target_index];
        }
        return dynamic_ref.bool_params[target_index];
    }
    if (negated) {
        return !static_ref.bool_params[target_index];
    }
    return static_ref.bool_params[target_index];
}

void BoolQueryInstance::print_value() const
{
    std::cout << "None";
}

DoubleQueryInstance::~DoubleQueryInstance()
{

}

DoubleQueryInstance::DoubleQueryInstance(double v, unsigned int x)
{
    target_index = x;
    comparison_value = v;
}


QueryInstance* DoubleQueryInstance::create_copy() const
{
    return new DoubleQueryInstance(*this);
}

bool DoubleQueryInstance::evaluate(const StaticState& static_ref) const
{
    if (dynamic) {
        throw std::invalid_argument("Cannot evaluate a dynamic function with static portion of belief state.");
    }
    if (negated) {
        return !is_close(static_ref.double_params[target_index], comparison_value);
    }
    return is_close(static_ref.double_params[target_index], comparison_value);
}

bool DoubleQueryInstance::evaluate(const StaticState &static_ref, const DynamicState &dynamic_ref) const
{
    if (dynamic) {
        if (negated) {
            return !is_close(dynamic_ref.double_params[target_index], comparison_value);
        }
        return is_close(dynamic_ref.double_params[target_index], comparison_value);
    }
    if (negated) {
        return !is_close(static_ref.double_params[target_index], comparison_value);
    }
    return is_close(static_ref.double_params[target_index], comparison_value);
}

void DoubleQueryInstance::print_value() const
{
    std::cout << comparison_value;
}

IntegerQueryInstance::~IntegerQueryInstance()
{
}

IntegerQueryInstance::IntegerQueryInstance(int v, unsigned int x)
{
    target_index = x;
    comparison_value = v;
}

QueryInstance* IntegerQueryInstance::create_copy() const
{
    return new IntegerQueryInstance(*this);
}

bool IntegerQueryInstance::evaluate(const StaticState& static_ref) const
{
    if (dynamic) {
        throw std::invalid_argument("Cannot evaluate a dynamic function with static portion of belief state.");
    }
    if (negated) {
        return static_ref.integer_params[target_index] != comparison_value;
    }
    return static_ref.integer_params[target_index] == comparison_value;
}

bool IntegerQueryInstance::evaluate(const StaticState &static_ref, const DynamicState &dynamic_ref) const
{
    if (dynamic) {
        if (negated) {
            return dynamic_ref.integer_params[target_index] != comparison_value;
        }
        return dynamic_ref.integer_params[target_index] == comparison_value;
    }
    if (negated) {
        return static_ref.integer_params[target_index] != comparison_value;
    }
    return static_ref.integer_params[target_index] == comparison_value;
}

void IntegerQueryInstance::print_value() const
{
    std::cout << comparison_value;
}

StringQueryInstance::~StringQueryInstance()
{

}

StringQueryInstance::StringQueryInstance(const std::string& v, unsigned int x)
{
    target_index = x;
    comparison_value = v;
}

QueryInstance* StringQueryInstance::create_copy() const
{
    return new StringQueryInstance(*this);
}

bool StringQueryInstance::evaluate(const StaticState& static_ref) const
{
    if (dynamic) {
        throw std::invalid_argument("Cannot evaluate a dynamic function with static portion of belief state.");
    }
    if (negated) {
        return static_ref.string_params[target_index] != comparison_value;
    }
    return static_ref.string_params[target_index] == comparison_value;
}

bool StringQueryInstance::evaluate(const StaticState &static_ref, const DynamicState &dynamic_ref) const
{
    if (dynamic) {
        if (negated) {
            return dynamic_ref.string_params[target_index] != comparison_value;
        }
        return dynamic_ref.string_params[target_index] == comparison_value;
    }
    if (negated) {
        return static_ref.string_params[target_index] != comparison_value;
    }
    return static_ref.string_params[target_index] == comparison_value;
}

void StringQueryInstance::print_value() const
{
    std::cout << comparison_value;
}