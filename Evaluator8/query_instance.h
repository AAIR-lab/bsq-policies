#ifndef EVAL8_QUERY_INSTANCE
#define EVAL8_QUERY_INSTANCE

#include "dynamic_state.h"
#include "static_state.h"

struct QueryInstance {
	virtual ~QueryInstance();
	unsigned int target_index = 0u;
	bool negated = false;
	bool dynamic = false;
	bool full_observable = false;
	virtual QueryInstance* create_copy() const = 0;
	virtual bool evaluate(const StaticState&) const = 0;
	virtual bool evaluate(const StaticState&, const DynamicState&) const = 0;
	virtual void print_value() const = 0;
};

struct BoolQueryInstance : public QueryInstance {
	~BoolQueryInstance();
	BoolQueryInstance(unsigned int);
	QueryInstance* create_copy() const;
	bool evaluate(const StaticState&) const;
	bool evaluate(const StaticState&, const DynamicState&) const;
	void print_value() const;
};

struct DoubleQueryInstance : public QueryInstance {
	~DoubleQueryInstance();
	DoubleQueryInstance(double, unsigned int);
	double comparison_value = 0.0;
	QueryInstance* create_copy() const;
	bool evaluate(const StaticState&) const;
	bool evaluate(const StaticState&, const DynamicState&) const;
	void print_value() const;
};

struct IntegerQueryInstance : public QueryInstance {
	~IntegerQueryInstance();
	IntegerQueryInstance(int, unsigned int);
	int comparison_value = 0;
	QueryInstance* create_copy() const;
	bool evaluate(const StaticState&) const;
	bool evaluate(const StaticState&, const DynamicState&) const;
	void print_value() const;
};

struct StringQueryInstance : public QueryInstance {
	~StringQueryInstance();
	StringQueryInstance(const std::string&, unsigned int);
	std::string comparison_value;
	QueryInstance* create_copy() const;
	bool evaluate(const StaticState&) const;
	bool evaluate(const StaticState&, const DynamicState&) const;
	void print_value() const;
};

#endif