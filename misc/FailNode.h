#ifndef FAILNODE_H
#define FAILNODE_H

#include <sofa/simulation/Node.h>

namespace sofa {
namespace simulation {

// throws on every method. for testing purpose.
class FailNode : public Node {
public:
	
    void doExecuteVisitor(simulation::Visitor* action, bool precomputedOrder=false) override;

	static void fail();

	void* getObject(const sofa::core::objectmodel::ClassInfo& class_info, 
					const sofa::core::objectmodel::TagSet& tags, 
					SearchDirection dir = SearchUp) const override;

    Node* findCommonParent( simulation::Node* node2 ) override;
	
	virtual void* getObject(const sofa::core::objectmodel::ClassInfo& class_info, 
                            const std::string& path) const override;

	void getObjects(const sofa::core::objectmodel::ClassInfo& class_info, 
							GetObjectsCallBack& container, 
							const sofa::core::objectmodel::TagSet& tags, 
							SearchDirection dir = SearchUp) const override;

    virtual Node::SPtr createChild(const std::string& nodeName) override;

	Parents getParents() const override;
	Children getChildren() const override;

    /// returns number of parents
    size_t getNbParents() const override;

    /// return the first parent (returns NULL if no parent)
    BaseNode* getFirstParent() const override;
	
	  /// Add a child node
    void doAddChild(BaseNode::SPtr node) override;

    /// Remove a child node
    void doRemoveChild(BaseNode::SPtr node) override;

    /// Move a node from another node
    void doMoveChild(BaseNode::SPtr node, BaseNode::SPtr prev_parent) override;

    /// Add a generic object
     bool doAddObject(core::objectmodel::BaseObject::SPtr obj, bool atEnd=true) override;

    /// Remove a generic object
    bool doRemoveObject(core::objectmodel::BaseObject::SPtr obj) override;

    /// Move an object from a node to another node
    using Node::doMoveObject;
    void doMoveObject(core::objectmodel::BaseObject::SPtr obj);

    /// Test if the given node is a parent of this node.
    bool hasParent(const BaseNode* node) const override; 

    /// Test if the given node is an ancestor of this node.
    /// An ancestor is a parent or (recursively) the parent of an ancestor.
    bool hasAncestor(const BaseNode* node) const override;
    bool hasAncestor(const BaseContext* context) const override;

    /// Remove the current node from the graph: depending on the type of Node, it can have one or several parents.
    void detachFromGraph() override;

    /// Get this node context
    BaseContext* getContext() override;

    /// Get this node context
    const BaseContext* getContext() const override;

    /// Return the full path name of this node
    std::string getPathName() const override;

    /// Return the path from this node to the root node
    std::string getRootPath() const override;

    Base* findLinkDestClass(const core::objectmodel::BaseClass* destType,
									const std::string& path, 
                                    const BaseLink* link) override;

	

}; 

}
}

#endif

