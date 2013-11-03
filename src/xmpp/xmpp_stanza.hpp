#ifndef XMPP_STANZA_INCLUDED
# define XMPP_STANZA_INCLUDED

#include <unordered_map>
#include <string>
#include <vector>

#include <expatpp.h>

/**
 * Raised on operator[] when the attribute does not exist
 */
class AttributeNotFound: public std::exception
{
};

/**
 * Represent an XML node. It has
 * - A parent XML node (in the case of the first-level nodes, the parent is
     nullptr)
 * - zero, one or more children XML nodes
 * - A name
 * - attributes
 * - inner data (inside the node)
 * - tail data (just after the node)
 */
class XmlNode
{
public:
  explicit XmlNode(const std::string& name, XmlNode* parent);
  explicit XmlNode(const std::string& name);
  XmlNode(XmlNode&& node):
    name(std::move(node.name)),
    parent(std::move(node.parent)),
    closed(std::move(node.closed)),
    attributes(std::move(node.attributes)),
    children(std::move(node.children)),
    inner(std::move(node.inner)),
    tail(std::move(node.tail))
  {
    node.parent = nullptr;
  }

  ~XmlNode();

  void delete_all_children();
  void set_attribute(const std::string& name, const std::string& value);
  /**
   * Set the content of the tail, that is the text just after this node
   */
  void set_tail(const std::string& data);
  /**
   * Set the content of the inner, that is the text inside this node
   */
  void set_inner(const std::string& data);
  void add_child(XmlNode* child);
  void add_child(XmlNode&& child);
  XmlNode* get_last_child() const;
  /**
   * Mark this node as closed, nothing else
   */
  void close();
  XmlNode* get_parent() const;
  const std::string& get_name() const;
  /**
   * Serialize the stanza into a string
   */
  std::string to_string() const;
  void display() const;
  /**
   * Whether or not this node has at least one child (if not, this is a leaf
   * node)
   */
  bool has_children() const;
  /**
   * Gets the value for the given attribute, raises AttributeNotFound if the
   * node as no such attribute.
   */
  const std::string& operator[](const std::string& name) const;
  /**
   * Use this to set an attribute's value, like node["id"] = "12";
   */
  std::string& operator[](const std::string& name);

private:
  std::string name;
  XmlNode* parent;
  bool closed;
  std::unordered_map<std::string, std::string> attributes;
  std::vector<XmlNode*> children;
  std::string inner;
  std::string tail;

  XmlNode(const XmlNode&) = delete;
  XmlNode& operator=(const XmlNode&) = delete;
  XmlNode& operator=(XmlNode&&) = delete;
};

/**
 * An XMPP stanza is just an XML node of level 2 in the XMPP document (the
 * level 1 ones are the <stream::stream/>, and the ones about 2 are just the
 * content of the stanzas)
 */
typedef XmlNode Stanza;

#endif // XMPP_STANZA_INCLUDED