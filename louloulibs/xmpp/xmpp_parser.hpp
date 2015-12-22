#ifndef XMPP_PARSER_INCLUDED
# define XMPP_PARSER_INCLUDED

#include <xmpp/xmpp_stanza.hpp>

#include <functional>

#include <expat.h>

/**
 * A SAX XML parser that builds XML nodes and spawns events when a complete
 * stanza is received (an element of level 2), or when the document is
 * opened/closed (an element of level 1)
 *
 * After a stanza_event has been spawned, we delete the whole stanza. This
 * means that even with a very long document (in XMPP the document is
 * potentially infinite), the memory is never exhausted as long as each
 * stanza is reasonnably short.
 *
 * The element names generated by expat contain the namespace of the
 * element, a colon (':') and then the actual name of the element.  To get
 * an element "x" with a namespace of "http://jabber.org/protocol/muc", you
 * just look for an XmlNode named "http://jabber.org/protocol/muc:x"
 *
 * TODO: enforce the size-limit for the stanza (limit the number of childs
 * it can contain). For example forbid the parser going further than level
 * 20 (arbitrary number here), and each XML node to have more than 15 childs
 * (arbitrary number again).
 */
class XmppParser
{
public:
  explicit XmppParser();
  ~XmppParser();

public:
  /**
   * Init the XML parser and install the callbacks
   */
  void init_xml_parser();
  /**
   * Feed the parser with some XML data
   */
  int feed(const char* data, const int len, const bool is_final);
  /**
   * Parse the data placed in the parser buffer
   */
  int parse(const int size, const bool is_final);
  /**
   * Reset the parser, so it can be used from scratch afterward
   */
  void reset();
  /**
   * Get a buffer provided by the xml parser.
   */
  void* get_buffer(const size_t size) const;
  /**
   * Add one callback for the various events that this parser can spawn.
   */
  void add_stanza_callback(std::function<void(const Stanza&)>&& callback);
  void add_stream_open_callback(std::function<void(const XmlNode&)>&& callback);
  void add_stream_close_callback(std::function<void(const XmlNode&)>&& callback);

  /**
   * Called when a new XML element has been opened. We instanciate a new
   * XmlNode and set it as our current node. The parent of this new node is
   * the previous "current" node. We have all the element's attributes in
   * this event.
   *
   * We spawn a stream_event with this node if this is a level-1 element.
   */
  void start_element(const XML_Char* name, const XML_Char** attribute);
  /**
   * Called when an XML element has been closed. We close the current_node,
   * set our current_node as the parent of the current_node, and if that was
   * a level-2 element we spawn a stanza_event with this node.
   *
   * And we then delete the stanza (and everything under it, its children,
   * attribute, etc).
   */
  void end_element(const XML_Char* name);
  /**
   * Some inner or tail data has been parsed
   */
  void char_data(const XML_Char* data, const size_t len);
  /**
   * Calls all the stanza_callbacks one by one.
   */
  void stanza_event(const Stanza& stanza) const;
  /**
   * Calls all the stream_open_callbacks one by one. Note: the passed node is not
   * closed yet.
   */
  void stream_open_event(const XmlNode& node) const;
  /**
   * Calls all the stream_close_callbacks one by one.
   */
  void stream_close_event(const XmlNode& node) const;

private:
  /**
   * Expat structure.
   */
  XML_Parser parser;
  /**
   * The current depth in the XML document
   */
  size_t level;
  /**
   * The deepest XML node opened but not yet closed (to which we are adding
   * new children, inner or tail)
   */
  XmlNode* current_node;
  /**
   * The root node has no parent, so we keep it here: the XmppParser object
   * is its owner.
   */
  std::unique_ptr<XmlNode> root;
  /**
   * A list of callbacks to be called on an *_event, receiving the
   * concerned Stanza/XmlNode.
   */
  std::vector<std::function<void(const Stanza&)>> stanza_callbacks;
  std::vector<std::function<void(const XmlNode&)>> stream_open_callbacks;
  std::vector<std::function<void(const XmlNode&)>> stream_close_callbacks;

  XmppParser(const XmppParser&) = delete;
  XmppParser& operator=(const XmppParser&) = delete;
};

#endif // XMPP_PARSER_INCLUDED
