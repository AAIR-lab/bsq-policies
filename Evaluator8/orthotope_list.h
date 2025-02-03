#ifndef EVAL7_ORTHOTOPE_LIST
#define EVAL7_ORTHOTOPE_LIST

#include<list>
#include "orthotope_kd_tree.h"

struct Orthotope {
	Orthotope() {};
	Orthotope(const std::vector<DimensionRange>&);
	Orthotope(const std::vector<DimensionRange>&, float);
	std::vector<DimensionRange> dimensions;
	float volume = 0.0f;

	void and_update(const Orthotope&);
	void not_update(const Orthotope&, Orthotope&) const;
	void not_update(const Orthotope&, std::list<Orthotope>&) const;
	bool dominated_by(const Orthotope&) const;
	bool overlaps_with(const Orthotope&) const;
	bool is_satisfiable() const;
	bool is_satisfied(const std::vector<float>&) const;
	void update_volume();
};

class OrthotopeList {
public:
	OrthotopeList();
	OrthotopeList(const std::vector<DimensionRange>&);
	OrthotopeList(const Orthotope&);
	void clear();
	void and_update(const Orthotope&);
	void and_update(const OrthotopeList&);
	void construct_ranges(std::vector<std::pair<float,float>>&) const;
	void or_update(const Orthotope&);
	void or_update(const OrthotopeList&);
	void not_update(const Orthotope&);
	void not_update(const OrthotopeList&);
	float get_total_volume() const;
	bool has_overlap(const OrthotopeList&) const;
	bool is_satisfiable() const;
	bool is_satisfied(const std::vector<float>&) const;
	void print(std::ostream&) const;
	void set_parameter_value(unsigned int, float, unsigned int);
	bool uniform_sample_point(std::default_random_engine&, std::vector<float>&) const;
	void split(unsigned int, std::list<OrthotopeList>&);

private:
	static float calculate_n_volume(const std::vector<DimensionRange>&);
	std::list<Orthotope> orthotopes;
	float total_volume;
};

std::ostream& operator<< (std::ostream&, const OrthotopeList&);

#endif