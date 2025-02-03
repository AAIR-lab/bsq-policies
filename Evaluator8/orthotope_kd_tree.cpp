#include "orthotope_kd_tree.h"
#include "constants.h"
#include<list>
#include<iostream>

bool DimensionRange::operator==(const DimensionRange& ref) const
{
	if (ref.include_lower != include_lower) {
		return false;
	}
	if (ref.include_upper != include_upper) {
		return false;
	}
	if (ref.lower_bound != lower_bound) {
		return false;
	}
	if (ref.upper_bound != upper_bound) {
		return false;
	}
	return true;
}

void OrthotopeKDTreeNode::update_n_volume()
{
	n_volume = 1.0f;
	for (std::vector<DimensionRange>::iterator it = orthotope.begin(); it != orthotope.end() && n_volume > 0.0f; ++it) {
		n_volume *= (it->upper_bound - it->lower_bound) + MIN_SIZE_CONSTANT;
	}
}

void OrthotopeKDTreeNode::cut_branch()
{
	if (left) {
		left->cut_branch();
		delete left;
		left = NULL;
		right->cut_branch();
		delete right;
		right = NULL;
	}
}

OrthotopeKDTree::OrthotopeKDTree(const std::vector<DimensionRange>& first_ortho) :parameter_ranges(first_ortho), root(NULL), total_n_volume(0.0f)
{
	root = new OrthotopeKDTreeNode;
	root->orthotope = first_ortho;

	root->update_n_volume();
	if (root->n_volume <= 0.0f) {
		delete root;
		root = NULL;
	}
	else {
		total_n_volume = root->n_volume;
	}
}

OrthotopeKDTree::OrthotopeKDTree(const std::vector<DimensionRange>& parameter_ranges, const std::vector<DimensionRange>& first_orthotope) :parameter_ranges(parameter_ranges), root(NULL), total_n_volume(0.0f)
{
	root = new OrthotopeKDTreeNode;
	root->orthotope = first_orthotope;
	root->update_n_volume();
	if (root->n_volume <= 0.0f) {
		delete root;
		root = NULL;
	}
	else {
		total_n_volume = root->n_volume;
	}
}

OrthotopeKDTree::OrthotopeKDTree(const OrthotopeKDTree& ref) :parameter_ranges(ref.parameter_ranges), root(NULL), total_n_volume(ref.total_n_volume)
{
	if (!ref.root) {
		return;
	}

	root = new OrthotopeKDTreeNode;
	std::list<OrthotopeKDTreeNode*> ptr_stack(1, root), ref_ptr_stack(1, ref.root);
	OrthotopeKDTreeNode* current = NULL, * ref_current = NULL;

	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();
		ref_current = ref_ptr_stack.back();
		ref_ptr_stack.pop_back();

		current->n_volume = ref_current->n_volume;
		if (ref_current->left) {
			current->split_index = ref_current->split_index;
			current->threshold = ref_current->threshold;

			current->left = new OrthotopeKDTreeNode;
			current->left->parent = current;
			ptr_stack.push_back(current->left);
			ref_ptr_stack.push_back(ref_current->left);

			current->right = new OrthotopeKDTreeNode;
			current->right->parent = current;
			ptr_stack.push_back(current->right);
			ref_ptr_stack.push_back(ref_current->right);
		}
		else {
			current->orthotope = ref_current->orthotope;
		}
	}
}

OrthotopeKDTree::~OrthotopeKDTree()
{
	total_n_volume = 0.0f;
	if (!root) {
		return;
	}
	cut_branch(root);
	root = NULL;
}

OrthotopeKDTree& OrthotopeKDTree::operator= (const OrthotopeKDTree& ref)
{
	if (&ref == this) {
		return *this;
	}

	std::list<OrthotopeKDTreeNode*> ptr_stack, ref_ptr_stack(1, ref.root);
	OrthotopeKDTreeNode* current = NULL, * ref_current = NULL;

	if (!ref.root) {
		if (!root) {
			return *this;
		}
		cut_branch(root);
		root = NULL;
		return *this;
	}
	if (root) {
		if (root->left) {
			ptr_stack.push_back(root->left);
			ptr_stack.push_back(root->right);
			while (!ptr_stack.empty()) {
				current = ptr_stack.back();
				ptr_stack.pop_back();

				if (current->left) {
					ptr_stack.push_back(current->left);
					ptr_stack.push_back(current->right);
				}
				delete current;
				current = NULL;
			}
			root->left = NULL;
			root->right = NULL;
		}
	}
	else {
		root = new OrthotopeKDTreeNode;
	}

	ptr_stack.push_back(root);
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();
		ref_current = ref_ptr_stack.back();
		ref_ptr_stack.pop_back();

		current->n_volume = ref_current->n_volume;
		if (ref_current->left) {
			current->split_index = ref_current->split_index;
			current->threshold = ref_current->threshold;

			current->left = new OrthotopeKDTreeNode;
			current->left->parent = current;
			ptr_stack.push_back(current->left);
			ref_ptr_stack.push_back(ref_current->left);

			current->right = new OrthotopeKDTreeNode;
			current->right->parent = current;
			ptr_stack.push_back(current->right);
			ref_ptr_stack.push_back(ref_current->right);
		}
		else {
			current->orthotope = ref_current->orthotope;
		}
	}
	return *this;
}

// void OrthotopeKDTree::and_update(const std::vector<DimensionRange> &ref)
// {
// 	if (!root) {
// 		return;
// 	}
// 	std::list<OrthotopeKDTreeNode*> update_stack(1,root);
// 	OrthotopeKDTreeNode* current = NULL;

// 	while (!update_stack.empty()) {
// 		current = update_stack.back();
// 		update_stack.pop_back();
// 		if (current->left) {
// 			if (ref[current->split_index].upper_bound < current->threshold) {
// 				current->left->updated = false;
// 				cut_branch(current->right);
// 				current->right = NULL;
// 				update_stack.push_back(current->left);
// 			}
// 			else if (ref[current->split_index].lower_bound >= current->threshold) {
// 				current->right->updated = false;
// 				cut_branch(current->left);
// 				current->left = NULL;
// 				update_stack.push_back(current->right);
// 			}
// 			else {
// 				current->left->updated = false;
// 				current->right->updated = false;
// 				update_stack.push_back(current->left);
// 				update_stack.push_back(current->right);
// 			}
// 		}
// 		else {
// 			size_t current_index = current->orthotope.size();
// 			current->n_volume = 1.0f;
// 			while (current_index != 0u) {
// 				--current_index;
// 				if (current->orthotope[current_index].lower_bound < ref[current_index].lower_bound) {
// 					current->orthotope[current_index].lower_bound = ref[current_index].lower_bound;
// 					current->orthotope[current_index].include_lower = ref[current_index].include_lower;
// 				}
// 				if (current->orthotope[current_index].upper_bound > ref[current_index].upper_bound) {
// 					current->orthotope[current_index].upper_bound = ref[current_index].upper_bound;
// 					current->orthotope[current_index].include_upper = ref[current_index].include_upper;
// 				}
// 				if(current->orthotope[current_index].upper_bound > current->orthotope[current_index].lower_bound){
// 					current->n_volume *= (current->orthotope[current_index].upper_bound - current->orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
// 				}
// 				else if(current->orthotope[current_index].upper_bound == current->orthotope[current_index].lower_bound && current->orthotope[current_index].include_lower && current->orthotope[current_index].include_upper){
// 					current->n_volume *= (current->orthotope[current_index].upper_bound - current->orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
// 				}
// 				else{
// 					current->n_volume = -1.0f;
// 					break;
// 				}
// 			}
// 			current->updated = true;
// 			backpropagate_update(current->parent);
// 		}
// 	}
// 	update_root();
// }

void OrthotopeKDTree::and_update(const std::vector<DimensionRange>& ref)
{
	if (!root) {
		return;
	}
	if (root->and_update_re(ref)) {
		total_n_volume = root->n_volume;
	}
	else {
		delete root;
		root = NULL;
		total_n_volume = 0.0f;
	}
}

bool OrthotopeKDTreeNode::and_update_re(const std::vector<DimensionRange>& ref)
{
	if (left) {
		OrthotopeKDTreeNode* temp = NULL;
		if (threshold > ref[split_index].upper_bound) {
			right->cut_branch();
			delete right;
			right = NULL;
			if (left->and_update_re(ref)) {
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete left;
				left = NULL;
				return false;
			}
		}
		else if (threshold <= ref[split_index].lower_bound) {
			left->cut_branch();
			delete left;
			left = NULL;
			if (right->and_update_re(ref)) {
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete right;
				right = NULL;
				return false;
			}
		}
		else if (left->and_update_re(ref)) {
			if (right->and_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else {
			delete left;
			left = NULL;
			if (right->and_update_re(ref)) {
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete right;
				right = NULL;
				return false;
			}
		}
	}
	else {
		size_t current_index = orthotope.size();
		n_volume = 1.0f;
		while (current_index != 0u) {
			--current_index;
			if (orthotope[current_index].lower_bound < ref[current_index].lower_bound) {
				orthotope[current_index].lower_bound = ref[current_index].lower_bound;
				orthotope[current_index].include_lower = ref[current_index].include_lower;
			}
			if (orthotope[current_index].upper_bound > ref[current_index].upper_bound) {
				orthotope[current_index].upper_bound = ref[current_index].upper_bound;
				orthotope[current_index].include_upper = ref[current_index].include_upper;
			}
			if (orthotope[current_index].upper_bound > orthotope[current_index].lower_bound) {
				n_volume *= (orthotope[current_index].upper_bound - orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
			}
			else if (orthotope[current_index].upper_bound == orthotope[current_index].lower_bound && orthotope[current_index].include_lower && orthotope[current_index].include_upper) {
				n_volume *= (orthotope[current_index].upper_bound - orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
			}
			else {
				n_volume = -1.0f;
				break;
			}
		}
		return n_volume > 0.0f;
	}
}

// void OrthotopeKDTree::and_update(const OrthotopeKDTree &ref)
// {
// 	if(!root){
// 		return;
// 	}
// 	if(!ref.root){
// 		cut_branch(root);
// 		root = NULL;
// 		return;
// 	}
// 	std::list<std::pair<OrthotopeKDTreeNode*,OrthotopeKDTreeNode const*>> ptr_stack;
// 	OrthotopeKDTreeNode *current = NULL, *old_root = root;
// 	OrthotopeKDTreeNode const *ref_current = NULL;
// 	ptr_stack.emplace_back(root,ref.root);
// 	root = NULL;
// 	bool overlap = true;
// 	std::vector<DimensionRange> temp(parameter_ranges.size());
// 	while(!ptr_stack.empty()){
// 		current = ptr_stack.back().first;
// 		ref_current = ptr_stack.back().second;
// 		ptr_stack.pop_back();
// 		if(current->left){
// 			ptr_stack.emplace_back(current->left,ref_current);
// 			ptr_stack.emplace_back(current->right,ref_current);
// 		}
// 		else if(ref_current->left){
// 			ptr_stack.emplace_back(current,ref_current->left);
// 			ptr_stack.emplace_back(current,ref_current->right);
// 		}
// 		else{
// 			overlap = true;
// 			for(int i = 0; i < current->orthotope.size() && overlap; ++i){
// 				if(current->orthotope[i].lower_bound > ref_current->orthotope[i].lower_bound){
// 					temp[i].lower_bound = current->orthotope[i].lower_bound;
// 					temp[i].include_lower = current->orthotope[i].include_lower;
// 				}
// 				else{
// 					temp[i].lower_bound = ref_current->orthotope[i].lower_bound;
// 					temp[i].include_lower = ref_current->orthotope[i].include_lower;					
// 				}
// 				if(current->orthotope[i].upper_bound < ref_current->orthotope[i].upper_bound){
// 					temp[i].upper_bound = current->orthotope[i].upper_bound;
// 					temp[i].include_upper = current->orthotope[i].include_upper;
// 				}
// 				else{
// 					temp[i].upper_bound = ref_current->orthotope[i].upper_bound;
// 					temp[i].include_upper = ref_current->orthotope[i].include_upper;					
// 				}
// 				if(temp[i].upper_bound < temp[i].lower_bound){
// 					overlap = false;
// 					break;
// 				}
// 				else if(temp[i].upper_bound == temp[i].lower_bound && !(temp[i].include_upper && temp[i].include_lower)){
// 					overlap = false;
// 					break;
// 				}
// 			}
// 			if(overlap){
// 				if(root){
// 					or_update(temp);
// 				}
// 				else{
// 					root = new OrthotopeKDTreeNode;
// 					root->orthotope = temp;
// 					root->update_n_volume();
// 					total_n_volume = root->n_volume;
// 				}
// 			}
// 			else{

// 			}
// 		}
// 	}
// 	cut_branch(old_root);
// 	old_root = NULL;
// }

// void OrthotopeKDTree::and_update(const OrthotopeKDTree &ref)
// {

// 	if (!root) {
// 		return;
// 	}
// 	if (!ref.root) {
// 		cut_branch(root);
// 		root = NULL;
// 		return;
// 	}
// 	std::list<dual_tree_info> update_stack;
// 	unsigned int temp_index = 0u;
// 	float temp_threshold = 0.0f;
// 	dual_tree_info* current = NULL;
// 	update_stack.emplace_back(root, ref.root, ref.parameter_ranges);

// 	while (!update_stack.empty()) {
// 		current = &update_stack.back();
// 		if (current->current_loc->left) {
// 			temp_index = current->current_loc->split_index;
// 			temp_threshold = current->current_loc->threshold;
// 			update_stack.emplace_back(current->current_loc->right, current->ref_current_loc, current->restrictions);
// 			update_stack.back().restrictions[temp_index].first = temp_threshold;
// 			update_stack.back().current_loc->updated = false;
// 			update_tree_reference(&update_stack.back());
// 			//++current;
// 			current->current_loc = current->current_loc->left;
// 			current->restrictions[temp_index].second = temp_threshold;
// 			current->current_loc->updated = false;
// 			update_tree_reference(current);
// 		}
// 		else if (current->ref_current_loc->left){
// 			if (current->current_loc->orthotope[current->ref_current_loc->split_index].upper_bound < current->ref_current_loc->threshold) {
// 				current->ref_current_loc = current->ref_current_loc->left;
// 				update_tree_reference(&*current);
// 			}
// 			else if (current->current_loc->orthotope[current->ref_current_loc->split_index].lower_bound >= current->ref_current_loc->threshold) {
// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(&*current);
// 			}
// 			else {
// 				current->current_loc->split_index = current->ref_current_loc->split_index;
// 				current->current_loc->threshold = current->ref_current_loc->threshold;
// 				current->current_loc->left = new OrthotopeKDTreeNode;
// 				current->current_loc->left->parent = current->current_loc;
// 				current->current_loc->left->updated = false;
// 				current->current_loc->left->orthotope = current->current_loc->orthotope;
// 				current->current_loc->right = new OrthotopeKDTreeNode;
// 				current->current_loc->right->parent = current->current_loc;
// 				current->current_loc->right->updated = false;
// 				current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 				update_stack.emplace_back(current->current_loc->left, current->ref_current_loc->left, current->restrictions);
// 				update_tree_reference(&update_stack.back());
// 				//++current;
// 				current->current_loc = current->current_loc->right;
// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(current);
// 			}
// 		}
// 		else {
// 			size_t current_index = current->current_loc->orthotope.size();
// 			current->current_loc->n_volume = 1.0f;
// 			while (current_index != 0u && current->current_loc->n_volume > 0.0f) {
// 				--current_index;
// 				if (current->current_loc->orthotope[current_index].first < current->ref_current_loc->orthotope[current_index].first) {
// 					current->current_loc->orthotope[current_index].first = current->ref_current_loc->orthotope[current_index].first;
// 				}
// 				if (current->current_loc->orthotope[current_index].second > current->ref_current_loc->orthotope[current_index].second) {
// 					current->current_loc->orthotope[current_index].second = current->ref_current_loc->orthotope[current_index].second;
// 				}
// 				current->current_loc->n_volume *= (current->current_loc->orthotope[current_index].second - current->current_loc->orthotope[current_index].first) + MIN_SIZE_CONSTANT;
// 			}
// 			current->current_loc->updated = true;
// 			backpropagate_update(current->current_loc->parent);
// 			update_stack.pop_back();
// 		}
// 	}
// 	update_root();
// }

void OrthotopeKDTree::and_update(const OrthotopeKDTree& ref)
{
	if (!root) {
		return;
	}
	if (!ref.root) {
		root->cut_branch();
		delete root;
		root = NULL;
		total_n_volume = 0.0f;
	}
	if (root->and_update_re(ref.root)) {
		total_n_volume = root->n_volume;
	}
	else {
		delete root;
		root = NULL;
		total_n_volume = 0.0f;
	}
}

bool OrthotopeKDTreeNode::and_update_re(OrthotopeKDTreeNode const* ref_tree)
{
	OrthotopeKDTreeNode* temp = NULL;
	if (left) {
		if (left->and_update_re(ref_tree)) {
			if (right->and_update_re(ref_tree)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else {
			delete left;
			left = NULL;
			if (right->and_update_re(ref_tree)) {
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete right;
				right = NULL;
				return false;
			}
		}
	}
	else if (ref_tree->left) {
		left = new OrthotopeKDTreeNode;
		left->orthotope = orthotope;
		right = new OrthotopeKDTreeNode;
		right->orthotope.swap(orthotope);
		split_index = ref_tree->split_index;
		threshold = ref_tree->threshold;
		if (left->and_update_re(ref_tree->left)) {
			if (right->and_update_re(ref_tree->right)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else {
			delete left;
			left = NULL;
			if (right->and_update_re(ref_tree->right)) {
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete right;
				right = NULL;
				return false;
			}
		}
	}
	else {
		size_t current_index = orthotope.size();
		n_volume = 1.0f;
		while (current_index != 0u) {
			--current_index;
			if (orthotope[current_index].lower_bound < ref_tree->orthotope[current_index].lower_bound) {
				orthotope[current_index].lower_bound = ref_tree->orthotope[current_index].lower_bound;
				orthotope[current_index].include_lower = ref_tree->orthotope[current_index].include_lower;
			}
			if (orthotope[current_index].upper_bound > ref_tree->orthotope[current_index].upper_bound) {
				orthotope[current_index].upper_bound = ref_tree->orthotope[current_index].upper_bound;
				orthotope[current_index].include_upper = ref_tree->orthotope[current_index].include_upper;
			}
			if (orthotope[current_index].upper_bound > orthotope[current_index].lower_bound) {
				n_volume *= (orthotope[current_index].upper_bound - orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
			}
			else if (orthotope[current_index].upper_bound == orthotope[current_index].lower_bound && orthotope[current_index].include_lower && orthotope[current_index].include_upper) {
				n_volume *= (orthotope[current_index].upper_bound - orthotope[current_index].lower_bound) + MIN_SIZE_CONSTANT;
			}
			else {
				n_volume = -1.0f;
				break;
			}
		}
		return n_volume > 0.0f;
	}
}

void OrthotopeKDTree::or_update(const std::vector<DimensionRange>& ref)
{
	if (!root) {
		return;
	}
	std::list<std::pair<OrthotopeKDTreeNode*, std::vector<DimensionRange>>> update_stack;
	update_stack.emplace_back(root, ref);
	std::list<std::pair<OrthotopeKDTreeNode*, std::vector<DimensionRange>>>::reverse_iterator current;
	int temp = -1;
	bool dominated = true;
	while (!update_stack.empty()) {
		current = update_stack.rbegin();
		if (current->first->left) {
			if (current->second[current->first->split_index].upper_bound < current->first->threshold) {
				current->first = current->first->left;
				current->first->updated = false;
			}
			else if (current->second[current->first->split_index].lower_bound >= current->first->threshold) {
				current->first = current->first->right;
				current->first->updated = false;
			}
			else {
				update_stack.emplace_back(current->first->right, current->second);
				++current;
				update_stack.back().second[current->first->split_index].lower_bound = current->first->threshold;
				update_stack.back().first->updated = false;
				current->second[current->first->split_index].upper_bound = current->first->threshold;
				current->first = current->first->left;
				current->first->updated = false;
			}
		}
		else {
			temp = -2;
			dominated = true;
			for (int i = 0; i < current->second.size(); ++i) {
				if (current->first->orthotope[i] == current->second[i]) {
					continue;
				}
				if (current->first->orthotope[i].lower_bound == current->second[i].upper_bound) {
					current->first->left = new OrthotopeKDTreeNode;
					current->first->left->parent = current->first;
					current->first->left->orthotope.swap(current->second);
					current->first->left->orthotope[i].include_upper = !current->first->orthotope[i].include_lower;
					current->first->left->update_n_volume();

					current->first->right = new OrthotopeKDTreeNode;
					current->first->right->parent = current->first;
					current->first->right->orthotope.swap(current->first->orthotope);
					current->first->right->n_volume = current->first->n_volume;
					//current->first->n_volume += current->first->left->n_volume;
					current->first->threshold = current->first->left->orthotope[i].upper_bound;
					current->first->split_index = i;
					temp = -1;
					break;
				}
				else if (current->first->orthotope[i].lower_bound > current->second[i].upper_bound) {
					current->first->left = new OrthotopeKDTreeNode;
					current->first->left->parent = current->first;
					current->first->left->orthotope.swap(current->second);
					current->first->left->orthotope[i].include_lower = !current->first->orthotope[i].include_upper;
					current->first->left->update_n_volume();

					current->first->right = new OrthotopeKDTreeNode;
					current->first->right->parent = current->first;
					current->first->right->orthotope.swap(current->first->orthotope);
					current->first->right->n_volume = current->first->n_volume;
					//current->first->n_volume += current->first->left->n_volume;
					current->first->threshold = current->first->left->orthotope[i].upper_bound;
					current->first->split_index = i;
					temp = -1;
					break;
				}
				else if (current->first->orthotope[i].upper_bound == current->second[i].lower_bound) {
					current->first->right = new OrthotopeKDTreeNode;
					current->first->right->parent = current->first;
					current->first->right->orthotope.swap(current->second);
					current->first->right->update_n_volume();

					current->first->left = new OrthotopeKDTreeNode;
					current->first->left->parent = current->first;
					current->first->left->orthotope.swap(current->first->orthotope);
					current->first->left->n_volume = current->first->n_volume;
					//current->first->n_volume += current->first->right->n_volume;
					current->first->threshold = current->first->right->orthotope[i].lower_bound;
					current->first->split_index = i;
					temp = -1;
					break;
				}
				else if (current->first->orthotope[i].upper_bound < current->second[i].lower_bound) {
					current->first->right = new OrthotopeKDTreeNode;
					current->first->right->parent = current->first;
					current->first->right->orthotope.swap(current->second);
					current->first->right->update_n_volume();

					current->first->left = new OrthotopeKDTreeNode;
					current->first->left->parent = current->first;
					current->first->left->orthotope.swap(current->first->orthotope);
					current->first->left->n_volume = current->first->n_volume;
					//current->first->n_volume += current->first->right->n_volume;
					current->first->threshold = current->first->right->orthotope[i].lower_bound;
					current->first->split_index = i;
					temp = -1;
					break;
				}
				else if (current->first->orthotope[i].lower_bound > current->second[i].lower_bound || current->first->orthotope[i].upper_bound <= current->second[i].upper_bound) {
					temp = i;
				}
				if (dominated && current->first->orthotope[i].lower_bound >= current->second[i].lower_bound && current->first->orthotope[i].upper_bound <= current->second[i].upper_bound) {
					dominated = true;
				}
				else {
					dominated = false;
				}
			}
			if (temp == -2) {
				current->first->updated = true;
				backpropagate_update(current->first->parent);
				update_stack.pop_back();
			}
			else if (temp < 0) {
				current->first->updated = true;
				backpropagate_update(current->first);
				update_stack.pop_back();
			}
			else if (dominated) {
				current->first->orthotope.swap(current->second);
				current->first->update_n_volume();
				current->first->updated = true;
				current->first->or_dominated = true;
				backpropagate_update(current->first->parent);
				update_stack.pop_back();
			}
			else if (current->first->orthotope[temp].upper_bound < current->second[temp].upper_bound) {
				current->first->split_index = temp;
				current->first->threshold = current->first->orthotope[temp].upper_bound;

				current->first->right = new OrthotopeKDTreeNode;
				current->first->right->parent = current->first;
				current->first->right->orthotope = current->second;
				current->first->right->orthotope[temp].lower_bound = current->first->threshold;
				current->first->right->orthotope[temp].include_lower = !current->first->orthotope[temp].include_upper;
				current->first->right->update_n_volume();

				current->first->left = new OrthotopeKDTreeNode;
				current->first->left->updated = false;
				current->first->left->parent = current->first;
				current->first->left->orthotope.swap(current->first->orthotope);
				current->first->left->n_volume = current->first->n_volume;
				current->second[temp].upper_bound = current->first->threshold; //
				current->first = current->first->left;

			}
			else {
				current->first->split_index = temp;
				current->first->threshold = current->first->orthotope[temp].lower_bound;

				current->first->left = new OrthotopeKDTreeNode;
				current->first->left->parent = current->first;
				current->first->left->orthotope = current->second;
				current->first->left->orthotope[temp].upper_bound = current->first->threshold;
				current->first->left->orthotope[temp].include_upper = !current->first->orthotope[temp].include_lower;
				current->first->left->update_n_volume();

				current->first->right = new OrthotopeKDTreeNode;
				current->first->right->updated = false;
				current->first->right->parent = current->first;
				current->first->right->orthotope.swap(current->first->orthotope);
				current->first->right->n_volume = current->first->n_volume;
				current->second[temp].lower_bound = current->first->threshold; //
				current->first = current->first->right;
			}

		}
	}
	update_root();
}

void OrthotopeKDTree::or_update(const OrthotopeKDTree& ref)
{
	if (!root) {
		*this = ref;
		return;
	}
	if (!ref.root) {
		return;
	}
	std::list<OrthotopeKDTreeNode*> ptr_stack(1, ref.root);
	OrthotopeKDTreeNode* current = NULL;
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();

		if (current->left) {
			ptr_stack.push_back(current->left);
			ptr_stack.push_back(current->right);
		}
		else {
			or_update(current->orthotope);
		}
	}
}

// void OrthotopeKDTree::or_update(const OrthotopeKDTree &ref)
// {
// 	if (!root) {
// 		*this = ref;
// 		return;
// 	}
// 	if (!ref.root) {
// 		return;
// 	}
// 	std::list<dual_tree_info> update_stack;
// 	unsigned int temp_index = 0u;
// 	float temp_threshold = 0.0f;
// 	std::vector<std::pair<float, float>> temp_orthotope;
// 	std::list<unsigned int> split_dimensions;
// 	bool dominated = false, ref_dominated = false, nonexistant_ortho = false;
// 	dual_tree_info* current = NULL;
// 	update_stack.emplace_back(root, ref.root, parameter_ranges);

// 	while (!update_stack.empty()) {
// 		current = &update_stack.back();
// 		if (current->current_loc->left) {
// 			temp_index = current->current_loc->split_index;
// 			temp_threshold = current->current_loc->threshold;
// 			update_stack.emplace_back(current->current_loc->right, current->ref_current_loc, current->restrictions);
// 			update_stack.back().restrictions[temp_index].first = temp_threshold;
// 			update_stack.back().current_loc->updated = false;
// 			update_tree_reference(&update_stack.back());
// 			current->current_loc = current->current_loc->left;
// 			current->restrictions[temp_index].second = temp_threshold;
// 			current->current_loc->updated = false;
// 			update_tree_reference(current);
// 		}
// 		else if (current->ref_current_loc->left) {
// 			if (current->current_loc->orthotope[current->ref_current_loc->split_index].second < current->ref_current_loc->threshold) {
// 				current->current_loc->left = new OrthotopeKDTreeNode;
// 				current->current_loc->left->parent = current->current_loc;
// 				current->current_loc->left->orthotope.swap(current->current_loc->orthotope);
// 				current->current_loc->left->updated = false;

// 				current->current_loc->right = new OrthotopeKDTreeNode;
// 				copy_branch(current->ref_current_loc->right, current->current_loc->right,current->restrictions);
// 				current->current_loc->right->parent = current->current_loc;

// 				current->current_loc->split_index = current->ref_current_loc->split_index;
// 				current->current_loc->threshold = current->ref_current_loc->threshold;
// 				current->current_loc = current->current_loc->left;

// 				current->ref_current_loc = current->ref_current_loc->left;
// 				update_tree_reference(&*current);

// 			}
// 			else if (current->current_loc->orthotope[current->ref_current_loc->split_index].first >= current->ref_current_loc->threshold) {
// 				current->current_loc->right = new OrthotopeKDTreeNode;
// 				current->current_loc->right->parent = current->current_loc;
// 				current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 				current->current_loc->right->updated = false;

// 				current->current_loc->left = new OrthotopeKDTreeNode;
// 				copy_branch(current->ref_current_loc->left, current->current_loc->left,current->restrictions);
// 				current->current_loc->left->parent = current->current_loc;

// 				current->current_loc->split_index = current->ref_current_loc->split_index;
// 				current->current_loc->threshold = current->ref_current_loc->threshold;
// 				current->current_loc = current->current_loc->right;

// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(&*current);
// 			}
// 			else {
// 				current->current_loc->split_index = current->ref_current_loc->split_index;
// 				current->current_loc->threshold = current->ref_current_loc->threshold;
// 				current->current_loc->left = new OrthotopeKDTreeNode;
// 				current->current_loc->left->parent = current->current_loc;
// 				current->current_loc->left->updated = false;
// 				current->current_loc->left->orthotope = current->current_loc->orthotope;
// 				current->current_loc->right = new OrthotopeKDTreeNode;
// 				current->current_loc->right->parent = current->current_loc;
// 				current->current_loc->right->updated = false;
// 				current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 				update_stack.emplace_back(current->current_loc->left, current->ref_current_loc->left, current->restrictions);
// 				update_tree_reference(&update_stack.back());
// 				//++current;
// 				current->current_loc = current->current_loc->right;
// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(current);
// 			}
// 		}
// 		else {
// 			dominated = true;
// 			ref_dominated = true;
// 			nonexistant_ortho = false;
// 			temp_orthotope = current->ref_current_loc->orthotope;
// 			for (int i = 0; i < current->ref_current_loc->orthotope.size(); ++i) {
// 				if (current->restrictions[i].first > temp_orthotope[i].first) {
// 					temp_orthotope[i].first = current->restrictions[i].first;
// 				}
// 				if (current->restrictions[i].second < temp_orthotope[i].second) {
// 					temp_orthotope[i].second = current->restrictions[i].second;
// 				}
// 				if (temp_orthotope[i].second < temp_orthotope[i].first) {
// 					nonexistant_ortho = true;
// 					break;
// 				}
// 			}



// 			if (nonexistant_ortho) {
// 				current->current_loc->updated = true;
// 				backpropagate_update(current->current_loc->parent);
// 				update_stack.pop_back();
// 				continue;
// 			}

// 			split_dimensions.clear();
// 			for (unsigned int i = 0; i < current->ref_current_loc->orthotope.size(); ++i) {

// 				if (current->current_loc->orthotope[i] == temp_orthotope[i]) {
// 					continue;
// 				}
// 				if (current->current_loc->orthotope[i].first > temp_orthotope[i].second) {
// 					current->current_loc->left = new OrthotopeKDTreeNode;
// 					current->current_loc->left->parent = current->current_loc;
// 					current->current_loc->left->orthotope.swap(temp_orthotope);
// 					current->current_loc->left->update_n_volume();

// 					current->current_loc->right = new OrthotopeKDTreeNode;
// 					current->current_loc->right->parent = current->current_loc;
// 					current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 					current->current_loc->right->n_volume = current->current_loc->n_volume;
// 					current->current_loc->threshold = current->current_loc->left->orthotope[i].second;
// 					current->current_loc->split_index = i;
// 					nonexistant_ortho = true;
// 					break;
// 				}
// 				else if (current->current_loc->orthotope[i].second < temp_orthotope[i].first) {
// 					current->current_loc->right = new OrthotopeKDTreeNode;
// 					current->current_loc->right->parent = current->current_loc;
// 					current->current_loc->right->orthotope.swap(temp_orthotope);
// 					current->current_loc->right->update_n_volume();

// 					current->current_loc->left = new OrthotopeKDTreeNode;
// 					current->current_loc->left->parent = current->current_loc;
// 					current->current_loc->left->orthotope.swap(current->current_loc->orthotope);
// 					current->current_loc->left->n_volume = current->current_loc->n_volume;
// 					current->current_loc->threshold = current->current_loc->right->orthotope[i].first;
// 					current->current_loc->split_index = i;
// 					nonexistant_ortho = true;
// 					break;
// 				}
// 				if(dominated){
// 					dominated = current->current_loc->orthotope[i].first < temp_orthotope[i].first&& current->current_loc->orthotope[i].second >= temp_orthotope[i].second;
// 				}
// 				if (ref_dominated) {
// 					ref_dominated = current->current_loc->orthotope[i].first >= temp_orthotope[i].first && current->current_loc->orthotope[i].second < temp_orthotope[i].second;
// 				}

// 				if (current->current_loc->orthotope[i].first >= temp_orthotope[i].first && current->current_loc->orthotope[i].first < temp_orthotope[i].second) {
// 					split_dimensions.push_back(i);
// 				}
// 				else if (current->current_loc->orthotope[i].second >= temp_orthotope[i].first && current->current_loc->orthotope[i].second < temp_orthotope[i].second) {
// 					split_dimensions.push_back(i);
// 				}
// 			}
// 			if (nonexistant_ortho) {
// 				backpropagate_update(current->current_loc);
// 				update_stack.pop_back();
// 				continue;
// 			}
// 			if (!dominated && !ref_dominated) {
// 				for (std::list<unsigned int>::iterator it = split_dimensions.begin(); it != split_dimensions.end(); ++it) {
// 					if (current->current_loc->orthotope[*it].first > temp_orthotope[*it].first) {

// 						current->current_loc->left = new OrthotopeKDTreeNode;
// 						current->current_loc->left->parent = current->current_loc;
// 						current->current_loc->left->orthotope = temp_orthotope;
// 						current->current_loc->left->orthotope[*it].second = current->current_loc->orthotope[*it].first;
// 						current->current_loc->left->update_n_volume();

// 						current->current_loc->right = new OrthotopeKDTreeNode;
// 						current->current_loc->right->parent = current->current_loc;
// 						temp_orthotope[*it].first = current->current_loc->orthotope[*it].first;
// 						current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 						current->current_loc->right->n_volume = current->current_loc->n_volume;
// 						current->current_loc->right->updated = false;

// 						current->current_loc = current->current_loc->right;
// 					}
// 					if (current->current_loc->orthotope[*it].second < temp_orthotope[*it].second) {

// 						current->current_loc->right = new OrthotopeKDTreeNode;
// 						current->current_loc->right->parent = current->current_loc;
// 						current->current_loc->right->orthotope = temp_orthotope;
// 						current->current_loc->right->orthotope[*it].first = current->current_loc->orthotope[*it].second;
// 						current->current_loc->right->update_n_volume();

// 						current->current_loc->left = new OrthotopeKDTreeNode;
// 						current->current_loc->left->parent = current->current_loc;
// 						temp_orthotope[*it].second = current->current_loc->orthotope[*it].second;
// 						current->current_loc->left->orthotope.swap(current->current_loc->orthotope);
// 						current->current_loc->left->n_volume = current->current_loc->n_volume;
// 						current->current_loc->left->updated = false;

// 						current->current_loc = current->current_loc->left;
// 					}
// 				}
// 			}
// 			//means one orthotop dominates the other.
// 			for (unsigned int i = 0; i < current->ref_current_loc->orthotope.size(); ++i) {
// 				if (current->current_loc->orthotope[i].first > temp_orthotope[i].first) {
// 					current->current_loc->orthotope[i].first = temp_orthotope[i].first;
// 				}
// 				if (current->current_loc->orthotope[i].second < temp_orthotope[i].second) {
// 					current->current_loc->orthotope[i].second = temp_orthotope[i].second;
// 				}
// 			}
// 			current->current_loc->update_n_volume();
// 			current->current_loc->updated = true;
// 			backpropagate_update(current->current_loc->parent);
// 			update_stack.pop_back();
// 			continue;
// 		}
// 	}
// 	update_root();
// }

float OrthotopeKDTree::get_total_volume() const
{
	return total_n_volume;
}

unsigned int OrthotopeKDTree::node_count() const
{
	unsigned int output = 0u;
	std::list<OrthotopeKDTreeNode*> tree_stack(1, root);
	OrthotopeKDTreeNode* current = NULL;
	while (!tree_stack.empty()) {
		current = tree_stack.back();
		tree_stack.pop_back();
		if (current) {
			++output;
			tree_stack.push_back(current->left);
			tree_stack.push_back(current->right);
		}
	}
	return output;
}

bool OrthotopeKDTree::is_satisfiable() const
{
	return root != NULL;
}

bool OrthotopeKDTree::is_satisfied(const std::vector<float>& ref) const
{
	if (!root) {
		return false;
	}
	OrthotopeKDTreeNode* current = root;
	while (current->left) {
		if (ref[current->split_index] >= current->threshold) {
			current = current->right;
		}
		else {
			current = current->left;
		}
	}
	size_t current_index = current->orthotope.size();
	while (current_index != 0u) {
		--current_index;
		if (current->orthotope[current_index].include_upper) {
			if (ref[current_index] > current->orthotope[current_index].upper_bound) {
				return false;
			}
		}
		else {
			if (ref[current_index] >= current->orthotope[current_index].upper_bound) {
				return false;
			}
		}
		if (current->orthotope[current_index].include_lower) {
			if (ref[current_index] < current->orthotope[current_index].lower_bound) {
				return false;
			}
		}
		else {
			if (ref[current_index] <= current->orthotope[current_index].lower_bound) {
				return false;
			}
		}
	}
	return true;
}

bool OrthotopeKDTree::uniform_sample_point(std::default_random_engine& generator, std::vector<float>& out) const
{
	if (!root) {
		return false;
	}
	if (total_n_volume <= 0.0f) {
		return false;
	}
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	float generated_value = dist(generator) * total_n_volume;
	OrthotopeKDTreeNode* current = root;
	while (current->left) {
		if (current->left->n_volume >= generated_value) {
			current = current->left;
		}
		else {
			generated_value -= current->left->n_volume;
			current = current->right;
		}
	}

	size_t current_index = current->orthotope.size();
	out.resize(current_index);
	while (current_index != 0u) {
		--current_index;
		//TODO: handle the case when bounds are not in.
		out[current_index] = dist(generator) * (current->orthotope[current_index].upper_bound - current->orthotope[current_index].lower_bound);
		out[current_index] += current->orthotope[current_index].lower_bound;
	}
	return true;
}

void OrthotopeKDTree::backpropagate_update(OrthotopeKDTreeNode* target) {
	OrthotopeKDTreeNode* current = NULL;
	while (target) {
		if (target->left) {
			if (!target->left->updated) {
				return;
			}
			if (target->left->n_volume <= 0.0f) {
				delete target->left;
				target->left = NULL;
			}
		}
		if (target->right) {
			if (!target->right->updated) {
				return;
			}
			if (target->right->n_volume <= 0.0f) {
				delete target->right;
				target->right = NULL;
			}
		}

		if (!target->left) {
			if (target->right) {
				current = target->right;
				target->n_volume = current->n_volume;
				target->orthotope.swap(current->orthotope);
				target->split_index = current->split_index;
				target->threshold = current->threshold;
				if (current->left) {
					target->left = current->left;
					current->left->parent = target;
					target->right = current->right;
					current->right->parent = target;
				}
				else {
					target->left = NULL;
					target->right = NULL;
				}
				delete current;
				current = NULL;
			}
			else {
				target->n_volume = 0.0f;
			}
		}
		else if (!target->right) {
			current = target->left;
			target->n_volume = current->n_volume;
			target->orthotope.swap(current->orthotope);
			target->split_index = current->split_index;
			target->threshold = current->threshold;
			if (current->left) {
				target->left = current->left;
				current->left->parent = target;
				target->right = current->right;
				current->right->parent = target;
			}
			else {
				target->left = NULL;
				target->right = NULL;
			}
			delete current;
			current = NULL;
		}
		else if (target->left->or_dominated) {
			if (target->right->or_dominated) {
				target->orthotope.swap(target->left->orthotope);
				target->orthotope[target->split_index].upper_bound = target->right->orthotope[target->split_index].upper_bound;
				target->n_volume = target->left->n_volume + target->right->n_volume;
				delete target->left;
				target->left = NULL;
				delete target->right;
				target->right = NULL;
				target->split_index = 0u;
				target->threshold = 0.0f;
				target->or_dominated = true;
			}
			else {
				target->left->or_dominated = false;
				target->n_volume = target->left->n_volume + target->right->n_volume;
			}
		}
		else if (target->right->or_dominated) {
			target->right->or_dominated = false;
			target->n_volume = target->left->n_volume + target->right->n_volume;
		}
		else {
			target->n_volume = target->left->n_volume + target->right->n_volume;
		}

		target->updated = true;
		target = target->parent;
	}
}

void OrthotopeKDTree::cut_branch(OrthotopeKDTreeNode* target)
{
	std::list<OrthotopeKDTreeNode*> ptr_stack(1, target);
	OrthotopeKDTreeNode* current = NULL;
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();

		if (current->left) {
			ptr_stack.push_back(current->left);
			ptr_stack.push_back(current->right);
		}
		delete current;
		current = NULL;
	}
}

// void OrthotopeKDTree::not_update(const std::vector<DimensionRange> &ref)
// {
// 	if (!root) {
// 		return;
// 	}
// 	std::list<OrthotopeKDTreeNode*> update_stack(1, root);
// 	OrthotopeKDTreeNode* current = NULL;
// 	bool dominated = true, not_split = true;
// 	while (!update_stack.empty()) {
// 		current = update_stack.back();
// 		update_stack.pop_back();
// 		if (current->left) {
// 			if (ref[current->split_index].upper_bound >= current->threshold) {
// 				current->right->updated = false;
// 				update_stack.push_back(current->right);
// 			}
// 			if (ref[current->split_index].lower_bound < current->threshold) {
// 				current->left->updated = false;
// 				update_stack.push_back(current->left);
// 			}
// 		}
// 		else {
// 			dominated = true;
// 			not_split = true;
// 			for (int i = 0; i < ref.size() && dominated && not_split; ++i) {
// 				if (current->orthotope[i].upper_bound <= ref[i].lower_bound) {
// 					not_split = false;
// 				}
// 				else if (current->orthotope[i].lower_bound >= ref[i].upper_bound) {
// 					not_split = false;
// 				}
// 				else if (current->orthotope[i].lower_bound < ref[i].lower_bound) {
// 					dominated = false;
// 					current->left = new OrthotopeKDTreeNode;
// 					current->left->parent = current;
// 					current->left->orthotope = current->orthotope;
// 					current->left->orthotope[i].upper_bound = ref[i].lower_bound;
// 					current->left->orthotope[i].include_upper = !ref[i].include_lower;
// 					current->left->update_n_volume();
// 					current->left->updated = false;
// 					update_stack.push_back(current->left);

// 					current->right = new OrthotopeKDTreeNode;
// 					current->right->parent = current;
// 					current->right->orthotope.swap(current->orthotope);
// 					current->right->orthotope[i].lower_bound = ref[i].lower_bound;
// 					current->right->orthotope[i].include_lower = ref[i].include_lower;
// 					current->right->update_n_volume();
// 					current->right->updated = false;
// 					update_stack.push_back(current->right);

// 					current->split_index = i;
// 					current->threshold = ref[i].lower_bound;
// 				}
// 				else if (current->orthotope[i].upper_bound > ref[i].upper_bound) {
// 					dominated = false;
// 					current->left = new OrthotopeKDTreeNode;
// 					current->left->parent = current;
// 					current->left->orthotope = current->orthotope;
// 					current->left->orthotope[i].upper_bound = ref[i].upper_bound;
// 					current->left->orthotope[i].include_upper = ref[i].include_upper;
// 					current->left->update_n_volume();
// 					current->left->updated = false;
// 					update_stack.push_back(current->left);

// 					current->right = new OrthotopeKDTreeNode;
// 					current->right->parent = current;
// 					current->right->orthotope.swap(current->orthotope);
// 					current->right->orthotope[i].lower_bound = ref[i].upper_bound;
// 					current->right->orthotope[i].include_lower = !ref[i].include_upper;
// 					current->right->update_n_volume();
// 					current->right->updated = false;
// 					update_stack.push_back(current->right);

// 					current->split_index = i;
// 					current->threshold = ref[i].upper_bound;
// 				}
// 			}
// 			if (!not_split) {
// 				current->updated = true;
// 				backpropagate_update(current->parent);
// 			}
// 			else if (dominated) {
// 				current->n_volume = 0.0f;
// 				current->updated = true;
// 				backpropagate_update(current->parent);
// 			}
// 		}
// 	}
// 	update_root();
// }

void OrthotopeKDTree::not_update(const std::vector<DimensionRange>& ref)
{
	if (!root) {
		return;
	}
	if (root->not_update_re(ref)) {
		total_n_volume = root->n_volume;
	}
	else {
		delete root;
		root = NULL;
		total_n_volume = 0.0f;
	}
}

bool OrthotopeKDTreeNode::not_update_re(const std::vector<DimensionRange>& ref)
{
	OrthotopeKDTreeNode* temp = NULL;
	int possible_split = -1;
	bool dominated = true;
	if (left) {
		if (threshold > ref[split_index].upper_bound) {
			if (left->not_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete left;
				left = NULL;
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else if (threshold <= ref[split_index].lower_bound) {
			if (right->not_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else if (left->not_update_re(ref)) {
			if (right->not_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
			}
			return true;
		}
		else {
			delete left;
			left = NULL;
			if (right->not_update_re(ref)) {
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				left = temp->left;
				right = temp->right;
				delete temp;
				temp = NULL;
				return true;
			}
			else {
				delete right;
				right = NULL;
				return false;
			}
		}
	}
	else {
		for (int i = 0; i < orthotope.size(); ++i) {
			if (ref[i].include_upper && orthotope[i].include_lower) {
				if (ref[i].upper_bound < orthotope[i].lower_bound) {
					return true;
				}
			}
			else {
				if (ref[i].upper_bound <= orthotope[i].lower_bound) {
					return true;
				}
			}
			if (ref[i].include_lower && orthotope[i].include_upper) {
				if (ref[i].lower_bound > orthotope[i].upper_bound) {
					return true;
				}
			}
			else {
				if (ref[i].lower_bound >= orthotope[i].upper_bound) {
					return true;
				}
			}
			if (ref[i].include_upper && orthotope[i].include_upper) {
				if (ref[i].upper_bound < orthotope[i].upper_bound) {
					possible_split = i;
				}
				if (dominated) {
					dominated = ref[i].upper_bound >= orthotope[i].upper_bound;
				}
			}
			else {
				if (ref[i].upper_bound <= orthotope[i].upper_bound) {
					possible_split = i;
				}
				if (dominated) {
					dominated = ref[i].upper_bound > orthotope[i].upper_bound;
				}
			}
			if ((ref[i].include_lower && orthotope[i].include_lower) || (!ref[i].include_lower && !orthotope[i].include_lower)) {
				if (ref[i].lower_bound > orthotope[i].lower_bound) {
					possible_split = i;
				}
				if (dominated) {
					dominated = ref[i].lower_bound <= orthotope[i].lower_bound;
				}
			}
			else {
				if (ref[i].lower_bound >= orthotope[i].lower_bound) {
					possible_split = i;
				}
				if (dominated) {
					dominated = ref[i].lower_bound < orthotope[i].lower_bound;
				}
			}
		}
		if (dominated || possible_split == -1) {
			return false;
		}

		dominated = false;
		if (ref[possible_split].include_upper && orthotope[possible_split].include_upper) {
			dominated = ref[possible_split].upper_bound < orthotope[possible_split].upper_bound;
		}
		else {
			dominated = ref[possible_split].upper_bound <= orthotope[possible_split].upper_bound;
		}

		split_index = possible_split;
		left = new OrthotopeKDTreeNode;
		left->orthotope = orthotope;
		right = new OrthotopeKDTreeNode;
		right->orthotope.swap(orthotope);

		if (dominated) {
			threshold = ref[possible_split].upper_bound;
			left->orthotope[split_index].upper_bound = threshold;
			left->orthotope[split_index].include_upper = ref[possible_split].include_upper;
			left->update_n_volume();
			right->orthotope[split_index].lower_bound = threshold;
			right->orthotope[split_index].include_lower = !ref[possible_split].include_upper;
			right->update_n_volume();
			if (left->not_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete left;
				left = NULL;
				temp = right;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				delete temp;
				temp = NULL;
				right = NULL;
			}
		}
		else {
			threshold = ref[possible_split].lower_bound;
			left->orthotope[split_index].upper_bound = threshold;
			left->orthotope[split_index].include_upper = !ref[possible_split].include_lower;
			left->update_n_volume();
			right->orthotope[split_index].lower_bound = threshold;
			right->orthotope[split_index].include_lower = ref[possible_split].include_lower;
			right->update_n_volume();
			if (right->not_update_re(ref)) {
				n_volume = left->n_volume + right->n_volume;
			}
			else {
				delete right;
				right = NULL;
				temp = left;
				orthotope.swap(temp->orthotope);
				n_volume = temp->n_volume;
				delete temp;
				temp = NULL;
				left = NULL;
			}
		}
		return true;
	}
}

void OrthotopeKDTree::not_update(const OrthotopeKDTree& ref)
{
	if (!root || !ref.root) {
		return;
	}

	std::list<OrthotopeKDTreeNode*> ptr_stack(1, ref.root);
	OrthotopeKDTreeNode* current = NULL;
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();

		if (current->left) {
			ptr_stack.push_back(current->left);
			ptr_stack.push_back(current->right);
		}
		else {
			not_update(current->orthotope);
		}
	}

}

// void OrthotopeKDTree::not_update(const OrthotopeKDTree & ref)
// {
// 	if (!root) {
// 		return;
// 	}
// 	if (!ref.root) {
// 		return;
// 	}

// 	std::list<dual_tree_info> update_stack;
// 	int temp_index = 0u;
// 	std::list<unsigned int> overlap_indices;
// 	float temp_threshold = 0.0f;
// 	bool dominated = true;
// 	dual_tree_info* current = NULL;
// 	update_stack.emplace_back(root, ref.root, ref.parameter_ranges);

// 	while (!update_stack.empty()) {
// 		current = &update_stack.back();
// 		if (current->current_loc->left) {
// 			temp_index = current->current_loc->split_index;
// 			temp_threshold = current->current_loc->threshold;
// 			update_stack.emplace_back(current->current_loc->right, current->ref_current_loc, current->restrictions);
// 			update_stack.back().restrictions[temp_index].first = temp_threshold;
// 			update_stack.back().current_loc->updated = false;
// 			update_tree_reference(&update_stack.back());
// 			//++current;
// 			current->current_loc = current->current_loc->left;
// 			current->restrictions[temp_index].second = temp_threshold;
// 			current->current_loc->updated = false;
// 			update_tree_reference(current);
// 		}
// 		else if (current->ref_current_loc->left) {
// 			if (current->current_loc->orthotope[current->ref_current_loc->split_index].second < current->ref_current_loc->threshold) {
// 				current->ref_current_loc = current->ref_current_loc->left;
// 				update_tree_reference(&*current);
// 			}
// 			else if (current->current_loc->orthotope[current->ref_current_loc->split_index].first >= current->ref_current_loc->threshold) {
// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(&*current);
// 			}
// 			else {
// 				current->current_loc->split_index = current->ref_current_loc->split_index;
// 				current->current_loc->threshold = current->ref_current_loc->threshold;
// 				current->current_loc->left = new OrthotopeKDTreeNode;
// 				current->current_loc->left->parent = current->current_loc;
// 				current->current_loc->left->updated = false;
// 				current->current_loc->left->orthotope = current->current_loc->orthotope;
// 				current->current_loc->right = new OrthotopeKDTreeNode;
// 				current->current_loc->right->parent = current->current_loc;
// 				current->current_loc->right->updated = false;
// 				current->current_loc->right->orthotope.swap(current->current_loc->orthotope);
// 				update_stack.emplace_back(current->current_loc->left, current->ref_current_loc->left, current->restrictions);
// 				update_tree_reference(&update_stack.back());
// 				//++current;
// 				current->current_loc = current->current_loc->right;
// 				current->ref_current_loc = current->ref_current_loc->right;
// 				update_tree_reference(current);
// 			}
// 		}
// 		else {
// 			overlap_indices.clear();
// 			dominated = true;
// 			for (size_t current_index = 0; current_index < current->current_loc->orthotope.size(); ++current_index) {
// 				if (current->current_loc->orthotope[current_index].second <= current->ref_current_loc->orthotope[current_index].second && current->current_loc->orthotope[current_index].first >= current->ref_current_loc->orthotope[current_index].first) {
// 					continue;
// 				}
// 				else {
// 					dominated = false;
// 					if (current->current_loc->orthotope[current_index].second <= current->ref_current_loc->orthotope[current_index].first) {
// 						overlap_indices.clear();
// 						break;
// 					}
// 					if (current->current_loc->orthotope[current_index].first > current->ref_current_loc->orthotope[current_index].second) {
// 						overlap_indices.clear();
// 						break;
// 					}
// 					overlap_indices.push_back(current_index);
// 				}
// 			}
// 			if (dominated) {
// 				current->current_loc->n_volume = -1.0f;
// 				current->current_loc->updated = true;
// 				backpropagate_update(current->current_loc->parent);
// 				update_stack.pop_back();
// 				continue;
// 			}
// 			if (overlap_indices.empty()) {
// 				current->current_loc->updated = true;
// 				backpropagate_update(current->current_loc->parent);
// 				update_stack.pop_back();
// 				continue;
// 			}
// 			while (!overlap_indices.empty()) {
// 				temp_index = overlap_indices.front();
// 				overlap_indices.pop_front();
// 				if (current->current_loc->orthotope[temp_index].first < current->ref_current_loc->orthotope[temp_index].first) {
// 					current->current_loc->split_index = temp_index;
// 					current->current_loc->threshold = current->ref_current_loc->orthotope[temp_index].first;

// 					current->current_loc->left = new OrthotopeKDTreeNode;
// 					current->current_loc->left->parent = current->current_loc;
// 					current->current_loc->left->orthotope = current->current_loc->orthotope;
// 					current->current_loc->left->orthotope[temp_index].second = current->ref_current_loc->orthotope[temp_index].first;
// 					current->current_loc->left->update_n_volume();

// 					current->current_loc->right = new OrthotopeKDTreeNode;
// 					current->current_loc->right->parent = current->current_loc;
// 					current->current_loc->right->orthotope = current->current_loc->orthotope;
// 					current->current_loc->right->orthotope[temp_index].first = current->ref_current_loc->orthotope[temp_index].first;
// 					current->current_loc->right->updated = false;

// 					current->current_loc = current->current_loc->right;
// 				}
// 				if (current->current_loc->orthotope[temp_index].second > current->ref_current_loc->orthotope[temp_index].second) {
// 					current->current_loc->split_index = temp_index;
// 					current->current_loc->threshold = current->ref_current_loc->orthotope[temp_index].second;

// 					current->current_loc->right = new OrthotopeKDTreeNode;
// 					current->current_loc->right->parent = current->current_loc;
// 					current->current_loc->right->orthotope = current->current_loc->orthotope;
// 					current->current_loc->right->orthotope[temp_index].first = current->ref_current_loc->orthotope[temp_index].second;
// 					current->current_loc->right->update_n_volume();

// 					current->current_loc->left = new OrthotopeKDTreeNode;
// 					current->current_loc->left->parent = current->current_loc;
// 					current->current_loc->left->orthotope.swap(current->current_loc->orthotope);
// 					current->current_loc->left->orthotope[temp_index].second = current->ref_current_loc->orthotope[temp_index].second;
// 					current->current_loc->left->updated = false;
// 					current->current_loc = current->current_loc->left;
// 				}
// 			}
// 			//This piece is guaranteed to completed overlap and must be removed.
// 			current->current_loc->n_volume = -1.0f;
// 			current->current_loc->updated = true;
// 			backpropagate_update(current->current_loc->parent);
// 			update_stack.pop_back();
// 		}
// 	}
// 	update_root();
// }

void OrthotopeKDTree::print(std::ostream& out_stream) const
{
	if (!root) {
		out_stream << "{}" << std::endl;
		return;
	}
	bool first = true;
	//TODO: reflect whether the bound or included or discluded.
	std::list<OrthotopeKDTreeNode*> ptr_stack(1, root);
	OrthotopeKDTreeNode* current = NULL;
	size_t current_index = 0;
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();
		if (current->left) {
			ptr_stack.push_back(current->left);
			ptr_stack.push_back(current->right);
		}
		else {
			if (!first) {
				out_stream << ",";
			}
			else {
				first = false;
			}
			out_stream << "{";
			for (current_index = 0; current_index < current->orthotope.size(); ++current_index) {
				out_stream << "(" << current->orthotope[current_index].lower_bound << " - " << current->orthotope[current_index].upper_bound << ")";
			}
			out_stream << "}";
		}
	}
}

void OrthotopeKDTree::update_root()
{
	OrthotopeKDTreeNode* current = NULL;
	if (!root) {
		return;
	}
	if (root->n_volume <= 0.0f) {
		total_n_volume = 0.0f;
		delete root;
		root = NULL;
		return;
	}
	root->or_dominated = false;
	total_n_volume = root->n_volume;
}

void OrthotopeKDTree::update_tree_reference(dual_tree_info* ref)
{
	while (ref->ref_current_loc->left) {
		if (ref->restrictions[ref->ref_current_loc->split_index].include_upper) {
			if (ref->restrictions[ref->ref_current_loc->split_index].upper_bound < ref->ref_current_loc->threshold) {
				ref->ref_current_loc = ref->ref_current_loc->left;
				continue;
			}
		}
		else {
			if (ref->restrictions[ref->ref_current_loc->split_index].upper_bound <= ref->ref_current_loc->threshold) {
				ref->ref_current_loc = ref->ref_current_loc->left;
				continue;
			}
		}

		if (ref->restrictions[ref->ref_current_loc->split_index].include_lower) {
			if (ref->restrictions[ref->ref_current_loc->split_index].lower_bound >= ref->ref_current_loc->threshold) {
				ref->ref_current_loc = ref->ref_current_loc->right;
				continue;
			}
		}
		else {
			if (ref->restrictions[ref->ref_current_loc->split_index].lower_bound >= ref->ref_current_loc->threshold) {
				ref->ref_current_loc = ref->ref_current_loc->right;
				continue;
			}
		}
		break;
	}
}

void OrthotopeKDTree::copy_branch(OrthotopeKDTreeNode const* ref_branch, OrthotopeKDTreeNode* target_branch, const std::vector<DimensionRange>& restriction)
{
	std::list<OrthotopeKDTreeNode*> ptr_stack(1, target_branch);
	std::list<OrthotopeKDTreeNode const*> ref_ptr_stack(1, ref_branch);
	OrthotopeKDTreeNode* current = NULL;
	OrthotopeKDTreeNode const* ref_current = NULL;
	while (!ptr_stack.empty()) {
		current = ptr_stack.back();
		ptr_stack.pop_back();
		ref_current = ref_ptr_stack.back();
		ref_ptr_stack.pop_back();

		if (ref_current->left) {
			if (restriction[ref_current->split_index].include_lower) {
				if (restriction[ref_current->split_index].lower_bound > ref_current->threshold) {
					ptr_stack.push_back(current);
					ref_ptr_stack.push_back(ref_current->right);
					continue;
				}
			}
			else {
				if (restriction[ref_current->split_index].lower_bound >= ref_current->threshold) {
					ptr_stack.push_back(current);
					ref_ptr_stack.push_back(ref_current->right);
					continue;
				}
			}
			if (restriction[ref_current->split_index].include_upper) {
				if (restriction[ref_current->split_index].upper_bound < ref_current->threshold) {
					ptr_stack.push_back(current);
					ref_ptr_stack.push_back(ref_current->left);
					continue;
				}
			}
			else {
				if (restriction[ref_current->split_index].upper_bound <= ref_current->threshold) {
					ptr_stack.push_back(current);
					ref_ptr_stack.push_back(ref_current->left);
					continue;
				}
			}
			current->split_index = ref_current->split_index;
			current->threshold = ref_current->threshold;
			current->left = new OrthotopeKDTreeNode;
			current->left->parent = current;
			current->left->updated = false;
			ptr_stack.push_back(current->left);
			ref_ptr_stack.push_back(ref_current->left);

			current->right = new OrthotopeKDTreeNode;
			current->right->parent = current;
			current->right->updated = false;
			ptr_stack.push_back(current->right);
			ref_ptr_stack.push_back(ref_current->right);
		}
		else {
			current->orthotope = ref_current->orthotope;
			current->n_volume = 1.0f;
			for (unsigned int i = 0; i < restriction.size() && current->n_volume > 0.0f; ++i) {
				if (current->orthotope[i].lower_bound < restriction[i].lower_bound) {
					current->orthotope[i].lower_bound = restriction[i].lower_bound;
					current->orthotope[i].include_lower = restriction[i].include_lower;
				}
				if (current->orthotope[i].upper_bound > restriction[i].upper_bound) {
					current->orthotope[i].upper_bound = restriction[i].upper_bound;
					current->orthotope[i].include_upper = restriction[i].include_upper;
				}
				if (current->orthotope[i].include_lower && current->orthotope[i].include_upper) {
					if (current->orthotope[i].upper_bound < current->orthotope[i].lower_bound) {
						current->n_volume = -1.0f;
						break;
					}
				}
				else {
					if (current->orthotope[i].upper_bound <= current->orthotope[i].lower_bound) {
						current->n_volume = -1.0f;
						break;
					}
				}
				current->n_volume *= (current->orthotope[i].upper_bound - current->orthotope[i].lower_bound) + MIN_SIZE_CONSTANT;
			}
			current->updated = true;
			backpropagate_update(current->parent);
		}
	}
}

OrthotopeKDTree::dual_tree_info::dual_tree_info(OrthotopeKDTreeNode* x, OrthotopeKDTreeNode* y, const std::vector<DimensionRange>& z)
	:current_loc(x), ref_current_loc(y), restrictions(z)
{

}
OrthotopeKDTree::dual_tree_info::dual_tree_info(OrthotopeKDTreeNode* x, OrthotopeKDTreeNode* y)
	:current_loc(x), ref_current_loc(y)
{

}

std::ostream& operator<< (std::ostream& out_stream, const OrthotopeKDTree& ref)
{
	ref.print(out_stream);
	return out_stream;
}
