#include "orthotope_list.h"
#include "utility.h"
#include "constants.h"
#include<iostream>
#include<iomanip>
#include<limits>
#include<math.h>

Orthotope::Orthotope(const std::vector<DimensionRange>& ref) :dimensions(ref)
{
    update_volume();
}

Orthotope::Orthotope(const std::vector<DimensionRange>& ref, float x) :dimensions(ref), volume(x)
{
    update_volume();
}

void Orthotope::and_update(const Orthotope& ref)
{
    for (size_t i = 0; i < ref.dimensions.size(); ++i) {
        if (is_close(dimensions[i].lower_bound, ref.dimensions[i].lower_bound) && dimensions[i].include_lower) {
            dimensions[i].include_lower = ref.dimensions[i].include_lower;
        }
        else if (dimensions[i].lower_bound < ref.dimensions[i].lower_bound) {
            dimensions[i].lower_bound = ref.dimensions[i].lower_bound;
            dimensions[i].include_lower = ref.dimensions[i].include_lower;
        }

        if (is_close(dimensions[i].upper_bound, ref.dimensions[i].upper_bound) && dimensions[i].include_upper) {
            dimensions[i].include_upper = ref.dimensions[i].include_upper;
        }
        else if (dimensions[i].upper_bound > ref.dimensions[i].upper_bound) {
            dimensions[i].upper_bound = ref.dimensions[i].upper_bound;
            dimensions[i].include_upper = ref.dimensions[i].include_upper;
        }
    }

    update_volume();
}

void Orthotope::not_update(const Orthotope& ref, std::list<Orthotope>& out) const
{
    if (!overlaps_with(ref)) {
        out.emplace_back(*this);
        return;
    }
    if (dominated_by(ref)) {
        return;
    }
    Orthotope temp(*this);
    for (size_t i = 0; i < ref.dimensions.size(); ++i) {
        if (ref.dimensions[i].include_lower) {
            if (temp.dimensions[i].lower_bound < ref.dimensions[i].lower_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].upper_bound = ref.dimensions[i].lower_bound;
                out.back().dimensions[i].include_upper = false;
                temp.dimensions[i].lower_bound = ref.dimensions[i].lower_bound;
                temp.dimensions[i].include_lower = true;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }
        else if (temp.dimensions[i].include_lower) {
            if (temp.dimensions[i].lower_bound <= ref.dimensions[i].lower_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].upper_bound = ref.dimensions[i].lower_bound;
                out.back().dimensions[i].include_upper = true;
                temp.dimensions[i].lower_bound = ref.dimensions[i].lower_bound;
                temp.dimensions[i].include_lower = false;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }
        else {
            if (temp.dimensions[i].lower_bound < ref.dimensions[i].lower_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].upper_bound = ref.dimensions[i].lower_bound;
                out.back().dimensions[i].include_upper = true;
                temp.dimensions[i].lower_bound = ref.dimensions[i].lower_bound;
                temp.dimensions[i].include_lower = false;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }

        if (ref.dimensions[i].include_upper) {
            if (temp.dimensions[i].upper_bound > ref.dimensions[i].upper_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].lower_bound = ref.dimensions[i].upper_bound;
                out.back().dimensions[i].include_lower = false;
                temp.dimensions[i].upper_bound = ref.dimensions[i].upper_bound;
                temp.dimensions[i].include_upper = true;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }
        else if (temp.dimensions[i].include_upper) {
            if (temp.dimensions[i].upper_bound >= ref.dimensions[i].upper_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].lower_bound = ref.dimensions[i].upper_bound;
                out.back().dimensions[i].include_lower = true;
                temp.dimensions[i].upper_bound = ref.dimensions[i].upper_bound;
                temp.dimensions[i].include_upper = false;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }
        else {
            if (temp.dimensions[i].upper_bound > ref.dimensions[i].upper_bound) {
                out.emplace_back(temp);
                out.back().dimensions[i].lower_bound = ref.dimensions[i].upper_bound;
                out.back().dimensions[i].include_lower = true;
                temp.dimensions[i].upper_bound = ref.dimensions[i].upper_bound;
                temp.dimensions[i].include_upper = false;
                out.back().update_volume();
                if (!out.back().is_satisfiable()) {
                    out.pop_back();
                }
            }
        }
    }

}

bool Orthotope::dominated_by(const Orthotope& ref) const
{
    for (size_t i = 0; i < ref.dimensions.size(); ++i) {
        if (ref.dimensions[i].include_lower) {
            if (dimensions[i].lower_bound < ref.dimensions[i].lower_bound) {
                return false;
            }
        }
        else if (dimensions[i].include_lower) {
            if (dimensions[i].lower_bound <= ref.dimensions[i].lower_bound) {
                return false;
            }
        }
        else {
            if (dimensions[i].lower_bound < ref.dimensions[i].lower_bound) {
                return false;
            }
        }

        if (ref.dimensions[i].include_upper) {
            if (dimensions[i].upper_bound > ref.dimensions[i].upper_bound) {
                return false;
            }
        }
        else if (dimensions[i].include_upper) {
            if (dimensions[i].upper_bound >= ref.dimensions[i].upper_bound) {
                return false;
            }
        }
        else {
            if (dimensions[i].upper_bound > ref.dimensions[i].upper_bound) {
                return false;
            }
        }
    }
    return true;
}

bool Orthotope::overlaps_with(const Orthotope& ref) const
{
    for (size_t i = 0; i < ref.dimensions.size(); ++i) {
        if (ref.dimensions[i].include_upper && dimensions[i].include_lower) {
            if (ref.dimensions[i].upper_bound < dimensions[i].lower_bound) {
                return false;
            }
        }
        else {
            if (ref.dimensions[i].upper_bound <= dimensions[i].lower_bound) {
                return false;
            }
        }

        if (ref.dimensions[i].include_lower && dimensions[i].include_upper) {
            if (ref.dimensions[i].lower_bound > dimensions[i].upper_bound) {
                return false;
            }
        }
        else {
            if (ref.dimensions[i].lower_bound >= dimensions[i].upper_bound) {
                return false;
            }
        }
    }
    return true;
}

bool Orthotope::is_satisfiable() const
{
    return volume > 0.0f;
}

bool Orthotope::is_satisfied(const std::vector<float>& ref) const
{
    for (size_t i = 0; i < ref.size(); ++i) {
        if (dimensions[i].include_upper) {
            if (dimensions[i].upper_bound < ref[i]) {
                return false;
            }
        }
        else {
            if (dimensions[i].upper_bound <= ref[i]) {
                return false;
            }
        }

        if (dimensions[i].include_lower) {
            if (dimensions[i].lower_bound > ref[i]) {
                return false;
            }
        }
        else {
            if (dimensions[i].lower_bound >= ref[i]) {
                return false;
            }
        }
    }
    return true;
}

void Orthotope::update_volume()
{
    volume = 1.0f;
    for (std::vector<DimensionRange>::iterator it = dimensions.begin(); it != dimensions.end(); ++it) {
        if (is_close(it->upper_bound, it->lower_bound)) {
            if (it->include_upper && it->include_lower) {
                volume *= MIN_SIZE_CONSTANT;
            }
            else {
                volume = -1.0f;
                break;
            }
        }
        else if (it->upper_bound > it->lower_bound) {
            volume *= (it->upper_bound - it->lower_bound) + MIN_SIZE_CONSTANT;
        }
        else {
            volume = -1.0f;
            break;
        }
    }
}
OrthotopeList::OrthotopeList() :total_volume(0.0f)
{
}
OrthotopeList::OrthotopeList(const std::vector<DimensionRange>& ref) :total_volume(0.0f)
{
    orthotopes.emplace_back(ref, total_volume);
    total_volume = orthotopes.back().volume;
    if (total_volume <= 0.0f) {
        orthotopes.clear();
        total_volume = 0.0f;
    }
}
OrthotopeList::OrthotopeList(const Orthotope &ref) :total_volume(ref.volume)
{
    if (total_volume > 0.0f) {
        orthotopes.emplace_back(ref);
    }
}
void OrthotopeList::and_update(const Orthotope& ref)
{
    std::list<Orthotope>::iterator it = orthotopes.begin();
    total_volume = 0.0f;
    while (it != orthotopes.end()) {
        it->and_update(ref);
        if (it->is_satisfiable()) {
            total_volume += it->volume;
            ++it;
        }
        else {
            it = orthotopes.erase(it);
        }
    }
}
void OrthotopeList::and_update(const OrthotopeList& ref)
{
    std::list<Orthotope> temp;
    total_volume = 0.0f;
    for (std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end(); ++it) {
        for (std::list<Orthotope>::const_iterator ref_it = ref.orthotopes.cbegin(); ref_it != ref.orthotopes.cend(); ++ref_it) {
            temp.emplace_back(*it);
            temp.back().and_update(*ref_it);
            if (!temp.back().is_satisfiable()) {
                temp.pop_back();
            }
            else {
                total_volume += temp.back().volume;
            }
        }
    }
    orthotopes.swap(temp);
}
void OrthotopeList::or_update(const Orthotope& ref)
{
    if (!ref.is_satisfiable()) {
        return;
    }
    OrthotopeList temp(ref);
    for (std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end() && temp.is_satisfiable(); ++it) {
        temp.not_update(*it);
    }
    if (temp.is_satisfiable()) {
        orthotopes.splice(orthotopes.end(), temp.orthotopes);
    }
}
void OrthotopeList::or_update(const OrthotopeList& ref)
{
    if (!ref.is_satisfiable()) {
        return;
    }
    OrthotopeList temp(ref);
    for (std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end() && temp.is_satisfiable(); ++it) {
        temp.not_update(*it);
    }
    if (temp.is_satisfiable()) {
        orthotopes.splice(orthotopes.end(), temp.orthotopes);
    }
}
void OrthotopeList::not_update(const Orthotope& ref)
{
    if (!ref.is_satisfiable()) {
        return;
    }
    std::list<Orthotope> temp;
    for (std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end(); ++it) {
        it->not_update(ref, temp);
    }
    orthotopes.swap(temp);
    total_volume = 0.0f;
    for (std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end(); ++it) {
        total_volume += it->volume;
    }
}

void OrthotopeList::not_update(const OrthotopeList& ref)
{
    if (!ref.is_satisfiable()) {
        return;
    }
    for (std::list<Orthotope>::const_iterator it = ref.orthotopes.cbegin(); it != ref.orthotopes.cend(); ++it) {
        not_update(*it);
    }
}

float OrthotopeList::get_total_volume() const
{
    return total_volume;
}

bool OrthotopeList::has_overlap(const OrthotopeList& ref) const
{
    if (orthotopes.empty() || ref.orthotopes.empty()) {
        return false;
    }
    bool overlap = false;
    std::list<Orthotope>::const_iterator orth_it = orthotopes.cbegin(), ref_it;
    for (; orth_it != orthotopes.cend() && !overlap; ++orth_it) {
        for (ref_it = ref.orthotopes.cbegin(); ref_it != ref.orthotopes.cend() && !overlap; ++ref_it) {
            overlap = true;
            for (int i = 0; i < orth_it->dimensions.size() && overlap; ++i) {
                //checks if the lower and upper bound are in the bounds.
                if (orth_it->dimensions[i].lower_bound > ref_it->dimensions[i].lower_bound && orth_it->dimensions[i].lower_bound < ref_it->dimensions[i].upper_bound) {
                    continue;
                }
                if (orth_it->dimensions[i].upper_bound > ref_it->dimensions[i].lower_bound && orth_it->dimensions[i].upper_bound < ref_it->dimensions[i].upper_bound) {
                    continue;
                }
                //edge cases where they only overlap on a single value.
                if (ref_it->dimensions[i].include_lower) {
                    if (orth_it->dimensions[i].include_lower && is_close(orth_it->dimensions[i].lower_bound, ref_it->dimensions[i].lower_bound)) {
                        continue;
                    }
                    if (orth_it->dimensions[i].include_upper && is_close(orth_it->dimensions[i].upper_bound, ref_it->dimensions[i].lower_bound)) {
                        continue;
                    }
                }
                if (ref_it->dimensions[i].include_upper) {
                    if (orth_it->dimensions[i].include_lower && is_close(orth_it->dimensions[i].lower_bound, ref_it->dimensions[i].upper_bound)) {
                        continue;
                    }
                    if (orth_it->dimensions[i].include_upper && is_close(orth_it->dimensions[i].upper_bound, ref_it->dimensions[i].upper_bound)) {
                        continue;
                    }
                }
                //Reaching here means there is no overlap.
                overlap = false;
            }
        }
    }
    return overlap;
}

bool OrthotopeList::is_satisfiable() const
{
    return !orthotopes.empty();
}
bool OrthotopeList::is_satisfied(const std::vector<float>& ref) const
{
    bool match = false;
    for (std::list<Orthotope>::const_iterator it = orthotopes.cbegin(); it != orthotopes.cend() && !match; ++it) {
        match = it->is_satisfied(ref);
    }
    return match;
}
void OrthotopeList::print(std::ostream& ref) const
{
    std::list<Orthotope>::const_iterator it = orthotopes.cbegin();
    while (it != orthotopes.cend()) {
        ref << "{";
        for (int i = 0; i < it->dimensions.size(); ++i) {
            if (it->dimensions[i].include_lower) {
                ref << "[";
            }
            else {
                ref << "(";
            }
            ref << std::setprecision(FLOAT_PRECISION) << it->dimensions[i].lower_bound << "," << it->dimensions[i].upper_bound;
            if (it->dimensions[i].include_upper) {
                ref << "]";
            }
            else {
                ref << ")";
            }
        }
        ref << "}";
        ++it;
    }
}
bool OrthotopeList::uniform_sample_point(std::default_random_engine& generator, std::vector<float>& out) const
{
    if (orthotopes.empty()) {
        return false;
    }

    float picked_value = std::uniform_real_distribution<>(0.0f, total_volume)(generator);
    std::list<Orthotope>::const_iterator it = orthotopes.cbegin();
    picked_value -= it->volume;
    while (it != orthotopes.cend() && picked_value > 0.0f) {
        ++it;
        picked_value -= it->volume;
    }

    float lower_bound = 0.0f, upper_bound = 0.0f;

    out.resize(it->dimensions.size());
    for (int i = 0; i < it->dimensions.size(); ++i) {
        lower_bound = it->dimensions[i].lower_bound;
        upper_bound = it->dimensions[i].upper_bound;
        if(!it->dimensions[i].include_lower){
            lower_bound += std::numeric_limits<float>::epsilon();
        }
        if(!it->dimensions[i].include_upper){
            upper_bound -= std::numeric_limits<float>::epsilon();
        }
        out[i] = std::uniform_real_distribution<>(lower_bound, upper_bound)(generator);
    }
    return true;
}

float OrthotopeList::calculate_n_volume(const std::vector<DimensionRange>& ref)
{
    float output = 1.0f;
    for (std::vector<DimensionRange>::const_iterator it = ref.cbegin(); it != ref.cend(); ++it) {
        output *= (it->upper_bound - it->lower_bound) + 0.000001f;
    }
    return output;
}

std::ostream& operator<< (std::ostream& ref_stream, const OrthotopeList& ref)
{
    ref.print(ref_stream);
    return ref_stream;
}

void OrthotopeList::set_parameter_value(unsigned int parameter_index, float comparison_value, unsigned int ineq_operator)
{
    std::list<Orthotope>::iterator it = orthotopes.begin();
    while (it != orthotopes.end()) {
        total_volume -= it->volume;
        switch (ineq_operator) {
        case 0u:
            //eq
            if (it->dimensions[parameter_index].include_upper) {
                if (it->dimensions[parameter_index].upper_bound < comparison_value) {
                    it = orthotopes.erase(it);
                    continue;
                }
            }
            else if (it->dimensions[parameter_index].upper_bound <= comparison_value) {
                it = orthotopes.erase(it);
                continue;
            }
            if (it->dimensions[parameter_index].include_lower) {
                if (it->dimensions[parameter_index].lower_bound > comparison_value) {
                    it = orthotopes.erase(it);
                    continue;
                }
            }
            else if (it->dimensions[parameter_index].lower_bound >= comparison_value) {
                it = orthotopes.erase(it);
                continue;
            }
            it->dimensions[parameter_index].include_upper = true;
            it->dimensions[parameter_index].include_lower = true;
            it->dimensions[parameter_index].lower_bound = comparison_value;
            it->dimensions[parameter_index].upper_bound = comparison_value;
            break;
        case 1u:
            //neq
            throw std::invalid_argument("neq currently not supported");
            break;
        case 2u:
            //gt
            if (is_close(it->dimensions[parameter_index].lower_bound,comparison_value)){
                it->dimensions[parameter_index].include_lower = false;
            }
            else if (it->dimensions[parameter_index].lower_bound < comparison_value) {
                it->dimensions[parameter_index].include_lower = false;
                it->dimensions[parameter_index].lower_bound = comparison_value;
            }
            break;
        case 3u:
            //lt
            if (is_close(it->dimensions[parameter_index].upper_bound, comparison_value)) {
                it->dimensions[parameter_index].include_upper = false;
            }
            else if (it->dimensions[parameter_index].upper_bound > comparison_value) {
                it->dimensions[parameter_index].include_upper = false;
                it->dimensions[parameter_index].upper_bound = comparison_value;
            }
            break;
        case 4u:
            //geq
            if (it->dimensions[parameter_index].lower_bound < comparison_value) {
                it->dimensions[parameter_index].include_lower = true;
                it->dimensions[parameter_index].lower_bound = comparison_value;
            }
            break;
        case 5u:
            //leq
            if (it->dimensions[parameter_index].upper_bound > comparison_value) {
                it->dimensions[parameter_index].include_upper = true;
                it->dimensions[parameter_index].upper_bound = comparison_value;
            }
            break;
        default:
            throw std::invalid_argument("inequality argument out of range");
            break;
        }
        it->update_volume();
        if (it->is_satisfiable()) {
            total_volume += it->volume;
            ++it;
        }
        else {
            it = orthotopes.erase(it);
        }
    }
}

void OrthotopeList::clear()
{
    orthotopes.clear();
}

void OrthotopeList::construct_ranges(std::vector<std::pair<float,float>> &out) const
{
    out.resize(orthotopes.front().dimensions.size());
    std::vector<std::pair<float,float>>::iterator out_it = out.begin();
    std::vector<DimensionRange>::const_iterator it = orthotopes.front().dimensions.cbegin();

    for(; out_it != out.end(); ++out_it, ++it){
        out_it->first = it->lower_bound;
        out_it->second = it->upper_bound;
    }
}

void OrthotopeList::split(unsigned int split_number, std::list<OrthotopeList> &output)
{
    output.clear();
    if(split_number < 2){
        return;
    }
    if(orthotopes.size() == 1){
        return;
    }

    Orthotope original = orthotopes.front();

    int clean_cuts = (int)std::floor(std::log2(float(split_number)));
    unsigned int current_dimension = 0u;
    float temp_average = 0.0f, temp = 0.0f;
    std::list<Orthotope>::iterator temp_it;

    //TODO: I am assuming that none of the dimensions have a length of 0!

    for(int i = 0; i < clean_cuts; ++i){
        for(std::list<Orthotope>::iterator it = orthotopes.begin(); it != orthotopes.end(); ++it){
            temp_average = (it->dimensions[current_dimension].upper_bound + it->dimensions[current_dimension].lower_bound) / 2.0f;
            orthotopes.push_front(*it);
            orthotopes.front().dimensions[current_dimension].upper_bound = temp_average;
            orthotopes.front().dimensions[current_dimension].include_upper = true;
            it->dimensions[current_dimension].lower_bound = temp_average;
            it->dimensions[current_dimension].include_lower = false;
        }
        ++current_dimension;
        if(current_dimension >= orthotopes.front().dimensions.size()){
            current_dimension = 0u;
        }
    }
    temp_it = orthotopes.begin();
    for(int remaining_cuts = split_number - pow(2,clean_cuts); remaining_cuts > 0; --remaining_cuts){
        current_dimension = 0;
        temp_average = temp_it->dimensions[0].upper_bound - temp_it->dimensions[0].lower_bound;
        for(int i = 1; i < temp_it->dimensions.size(); ++i){
            temp = temp_it->dimensions[i].upper_bound - temp_it->dimensions[i].lower_bound;
            if(temp > temp_average){
                current_dimension = i;
                temp_average = temp;
            }
        }

        temp_average = (temp_it->dimensions[current_dimension].upper_bound + temp_it->dimensions[current_dimension].lower_bound) / 2.0f;
        orthotopes.push_front(*temp_it);
        orthotopes.front().dimensions[current_dimension].upper_bound = temp_average;
        orthotopes.front().dimensions[current_dimension].include_upper = true;
        temp_it->dimensions[current_dimension].lower_bound = temp_average;
        temp_it->dimensions[current_dimension].include_lower = false;

        ++temp_it;
        if(temp_it == orthotopes.end()){
            temp_it = orthotopes.begin();
        }      
    }

    for(temp_it = orthotopes.begin(); temp_it != orthotopes.end(); ++temp_it){
        temp_it->update_volume();
        output.emplace_back(*temp_it);
    }

    orthotopes.clear();
    orthotopes.push_front(original);
}