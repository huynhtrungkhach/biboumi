#ifndef BRIDGE_INCLUDED
# define BRIDGE_INCLUDED

#include <irc/irc_client.hpp>
#include <bridge/colors.hpp>
#include <irc/irc_user.hpp>
#include <irc/iid.hpp>

#include <unordered_map>
#include <string>
#include <memory>

class XmppComponent;
class Poller;

/**
 * One bridge is spawned for each XMPP user that uses the component.  The
 * bridge spawns IrcClients when needed (when the user wants to join a
 * channel on a new server) and does the translation between the two
 * protocols.
 */
class Bridge
{
public:
  explicit Bridge(const std::string& user_jid, XmppComponent* xmpp, std::shared_ptr<Poller> poller);
  ~Bridge();
  /**
   * QUIT all connected IRC servers.
   */
  void shutdown(const std::string& exit_message);
  /**
   * Remove all inactive IrcClients
   */
  void clean();

  static Xmpp::body make_xmpp_body(const std::string& str);
  /***
   **
   ** From XMPP to IRC.
   **
   **/

  /**
   * Try to join an irc_channel, does nothing and return true if the channel
   * was already joined.
   */
  bool join_irc_channel(const Iid& iid, const std::string& username);
  void send_channel_message(const Iid& iid, const std::string& body);
  void send_private_message(const Iid& iid, const std::string& body, const std::string& type="PRIVMSG");
  void leave_irc_channel(Iid&& iid, std::string&& status_message);
  void send_irc_nick_change(const Iid& iid, const std::string& new_nick);
  void send_irc_kick(const Iid& iid, const std::string& target, const std::string& reason);
  void set_channel_topic(const Iid& iid, const std::string& subject);
  void send_xmpp_version_to_irc(const Iid& iid, const std::string& name, const std::string& version, const std::string& os);

  /***
   **
   ** From IRC to XMPP.
   **
   **/

  /**
   * Send a message corresponding to a server NOTICE, the from attribute
   * should be juste the server hostname@irc.component.
   */
  void send_xmpp_message(const std::string& from, const std::string& author, const std::string& msg);
  /**
   * Send the presence of a new user in the MUC.
   */
  void send_user_join(const std::string& hostname,
                      const std::string& chan_name,
                      const IrcUser* user,
                      const char user_mode,
                      const bool self);
  /**
   * Send the topic of the MUC to the user
   */
  void send_topic(const std::string& hostname, const std::string& chan_name, const std::string topic);
  /**
   * Send a MUC message from some participant
   */
  void send_message(const Iid& iid, const std::string& nick, const std::string& body, const bool muc);
  /**
   * Send a presence of type error, from a room. This is used to indicate
   * why joining a room failed.
   */
  void send_join_failed(const Iid& iid, const std::string& nick, const std::string& type, const std::string& condition, const std::string& text);
  /**
   * Send an unavailable presence from this participant
   */
  void send_muc_leave(Iid&& iid, std::string&& nick, const std::string& message, const bool self);
  /**
   * Send presences to indicate that an user old_nick (ourself if self ==
   * true) changed his nick to new_nick.  The user_mode is needed because
   * the xmpp presence needs ton contain the role and affiliation of the
   * user.
   */
  void send_nick_change(Iid&& iid,
                        const std::string& old_nick,
                        const std::string& new_nick,
                        const char user_mode,
                        const bool self);
  void kick_muc_user(Iid&& iid, const std::string& target, const std::string& reason, const std::string& author);
  void send_nickname_conflict_error(const Iid& iid, const std::string& nickname);
  /**
   * Send a role/affiliation change, matching the change of mode for that user
   */
  void send_affiliation_role_change(const Iid& iid, const std::string& target, const char mode);
  /**
   * Send an iq version request coming from nick%hostname@
   */
  void send_iq_version_request(const std::string& nick, const std::string& hostname);

  /**
   * Misc
   */
  std::string get_own_nick(const Iid& iid);
  /**
   * Get the number of server to which this bridge is connected or connecting.
   */
  size_t active_clients() const;

private:
  /**
   * Returns the client for the given hostname, create one (and use the
   * username in this case) if none is found, and connect that newly-created
   * client immediately.
   */
  IrcClient* get_irc_client(const std::string& hostname, const std::string& username);
  /**
   * This version does not create the IrcClient if it does not exist, and
   * returns nullptr in that case
   */
  IrcClient* get_irc_client(const std::string& hostname);
  /**
   * The JID of the user associated with this bridge. Messages from/to this
   * JID are only managed by this bridge.
   */
  const std::string user_jid;
  /**
   * One IrcClient for each IRC server we need to be connected to.
   * The pointer is shared by the bridge and the poller.
   */
  std::unordered_map<std::string, std::shared_ptr<IrcClient>> irc_clients;
  /**
   * A raw pointer, because we do not own it, the XMPP component owns us,
   * but we still need to communicate with it, when sending messages from
   * IRC to XMPP.
   */
  XmppComponent* xmpp;
  /**
   * Poller, to give it the IrcClients that we spawn, to make it manage
   * their sockets.
   */
  std::shared_ptr<Poller> poller;

  Bridge(const Bridge&) = delete;
  Bridge(Bridge&& other) = delete;
  Bridge& operator=(const Bridge&) = delete;
  Bridge& operator=(Bridge&&) = delete;
};

#endif // BRIDGE_INCLUDED
