#ifndef EVAL7_ORTHOTOPE_KD_TREE
#define EVAL7_ORTHOTOPE_KD_TREE

#include<random>
#include<vector>
#include<fstream>

struct DimensionRange {
	float lower_bound = 0.0f;
	float upper_bound = 0.0f;
	bool include_lower = true;
	bool include_upper = true;
	bool operator==(const DimensionRange&) const;
};

struct OrthotopeKDTreeNode {

	std::vector<DimensionRange> orthotope;
	unsigned int split_index = 0u;
	float threshold = 0.0f;
	OrthotopeKDTreeNode* left = NULL;
	OrthotopeKDTreeNode* right = NULL;
	OrthotopeKDTreeNode* parent = NULL;
	float n_volume = 0.0f;
	void update_n_volume();
	bool updated = true;
	bool or_dominated = false;
	bool temp_flag = false;

	void cut_branch();
	bool and_update_re(const std::vector<DimensionRange>&);
	bool and_update_re(OrthotopeKDTreeNode const*);
	//bool or_update_re(const std::vector<DimensionRange>&);
	//bool or_update_re(OrthotopeKDTreeNode const*,std::vector<DimensionRange>&);
	bool not_update_re(const std::vector<DimensionRange>&);
	//bool not_update_re(OrthotopeKDTreeNode const*,std::vector<DimensionRange>&);
};

class OrthotopeKDTree {
public:
	OrthotopeKDTree(const std::vector<DimensionRange>&);
	OrthotopeKDTree(const std::vector<DimensionRange>&, const std::vector<DimensionRange>&);
	OrthotopeKDTree(const OrthotopeKDTree&);
	~OrthotopeKDTree();
	OrthotopeKDTree& operator= (const OrthotopeKDTree&);
	void and_update(const std::vector<DimensionRange>&);
	void and_update(const OrthotopeKDTree&);
	void or_update(const std::vector<DimensionRange>&);
	void or_update(const OrthotopeKDTree&);
	void not_update(const std::vector<DimensionRange>&);
	void not_update(const OrthotopeKDTree&);
	float get_total_volume() const;
	unsigned int node_count() const;
	bool is_satisfiable() const;
	bool is_satisfied(const std::vector<float>&) const;
	void print(std::ostream&) const;
	bool uniform_sample_point(std::default_random_engine&, std::vector<float>&) const;

private:
	struct dual_tree_info {
		OrthotopeKDTreeNode* current_loc = NULL;
		OrthotopeKDTreeNode* ref_current_loc = NULL;
		std::vector<DimensionRange> restrictions;
		dual_tree_info(OrthotopeKDTreeNode* x, OrthotopeKDTreeNode* y);
		dual_tree_info(OrthotopeKDTreeNode* x, OrthotopeKDTreeNode* y, const std::vector<DimensionRange>& z);
	};
	static void cut_branch(OrthotopeKDTreeNode*);
	static void copy_branch(OrthotopeKDTreeNode const*, OrthotopeKDTreeNode*, const std::vector<DimensionRange>&);
	static void backpropagate_update(OrthotopeKDTreeNode*);
	void update_root();
	static void update_tree_reference(dual_tree_info*);

	std::vector<DimensionRange> parameter_ranges;
	OrthotopeKDTreeNode* root;
	float total_n_volume;
};

std::ostream& operator<< (std::ostream&, const OrthotopeKDTree&);

#endif
