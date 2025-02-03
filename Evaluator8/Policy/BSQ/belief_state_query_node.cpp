#include "belief_state_query_node.h"

BSQNode::BSQNode()
{

}

BSQNode::~BSQNode()
{
    parent = nullptr;
    if (instance_check) {
        delete instance_check;
        instance_check = nullptr;
    }
    for (std::vector< BSQNode*>::iterator it = children.begin(); it != children.end(); ++it) {
        delete* it;
        *it = nullptr;
    }
    children.clear();
}
