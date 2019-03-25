// Copyright (C) 2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SLAAC_CONFIG_H
#define SLAAC_CONFIG_H

#include <process/d_cfg_mgr.h>


namespace isc {
namespace slaac {

class SlaacConfig;
/// @brief Pointer to a configuration context.
typedef boost::shared_ptr<SlaacConfig> SlaacConfigPtr;

/// @brief Control Agent Configuration Context.
///
/// Implement the storage container for configuration context.
/// It provides a single enclosure for the storage of configuration parameters
/// and any other Control Agent specific information that needs to be accessible
/// during configuration parsing as well as to the application as a whole.
/// It is derived from the context base class, ConfigBase.
class SlaacConfig : public process::ConfigBase {
public:

    /// @brief Default constructor
    SlaacConfig();

    /// @brief Creates a clone of this context object.
    ///
    /// Note this method does not do deep copy the information about control sockets.
    /// That data is stored as ConstElementPtr (a shared pointer) to the actual data.
    ///
    /// @return A pointer to the new clone.
    virtual process::ConfigPtr clone() {
        return (process::ConfigPtr(new SlaacConfig(*this)));
    }

    /// @brief Returns information about control socket
    ///
    /// This method returns Element tree structure that describes the control
    /// socket (or null pointer if the socket is not defined for a particular
    /// server type). This information is expected to be compatible with
    /// data passed to @ref isc::config::CommandMgr::openCommandSocket.
    ///
    /// @param service server being controlled
    /// @return pointer to the Element that holds control-socket map (or NULL)
    isc::data::ConstElementPtr
    getControlSocketInfo(const std::string& service) const;

    /// @brief Sets information about the control socket
    ///
    /// This method stores Element tree structure that describes the control
    /// socket. This information is expected to be compatible with
    /// data passed to @ref isc::config::CommandMgr::openCommandSocket.
    ///
    /// @param control_socket Element that holds control-socket map
    /// @param service server being controlled
    void setControlSocketInfo(const isc::data::ConstElementPtr& control_socket,
                              const std::string& service);

    /// @brief Returns socket configuration summary in a textual format.
    std::string getControlSocketInfoSummary() const;

    /// @brief Returns non-const reference to configured hooks libraries.
    ///
    /// @return non-const reference to configured hooks libraries.
    isc::hooks::HooksConfig& getHooksConfig() {
        return (hooks_config_);
    }

    /// @brief Returns const reference to configured hooks libraries.
    ///
    /// @return const reference to configured hooks libraries.
    const isc::hooks::HooksConfig& getHooksConfig() const {
        return (hooks_config_);
    }

    /// @brief Unparse a configuration object
    ///
    /// Returns an element which must parse into the same object, i.e.
    /// @code
    /// for all valid config C parse(parse(C)->toElement()) == parse(C)
    /// @endcode
    ///
    /// @return a pointer to a configuration which can be parsed into
    /// the initial configuration object
    virtual isc::data::ElementPtr toElement() const;


    void setHopLimit(uint8_t hop_limit) {
        hop_limit_ = hop_limit;
    }

    uint8_t getHopLimit() const {
        return (hop_limit_);
    }

    void setManagedFlag(bool managed_flag) {
        managed_flag_ = managed_flag;
    }

    bool getManagedFlag() const {
        return (managed_flag_);
    }

    void setOtherFlag(bool other_flag) {
        other_flag_ = other_flag;
    }

    bool getOtherFlag() const {
        return (other_flag_);
    }

    void setRouterLifetime(uint16_t router_lifetime) {
        router_lifetime_ = router_lifetime;
    }

    uint16_t getRouterLifetime() const {
        return (router_lifetime_);
    }

    void setReachableTime(uint32_t reachable_time) {
        reachable_time_ = reachable_time;
    }

    uint32_t getReachableTime() const {
        return (reachable_time_);
    }

    void setRetransTimer(uint32_t retrans_timer) {
        retrans_timer_ = retrans_timer;
    }

    uint32_t getRetransTimer() const {
        return (retrans_timer_);
    }

    void setSrcLlAddr(bool src_ll_addr) {
        src_ll_addr_ = src_ll_addr;
    }

    bool getSrcLlAddr() const {
        return (src_ll_addr_);
    }

    void setMtu(uint32_t mtu) {
        mtu_ = mtu;
    }

    uint32_t getMtu() const {
        return (mtu_);
    }

    isc::data::ConstElementPtr getUnivRa() const {
        return (universal_ra_);
    }

    void setUnivRa(const isc::data::ConstElementPtr& universal_ra) {
        universal_ra_ = universal_ra;
    }

private:

    /// @brief Private copy constructor
    ///
    /// It is private to forbid anyone outside of this class to make copies.
    /// The only legal way to copy a context is to call @ref clone().
    ///
    /// @param orig the original context to copy from
    SlaacConfig(const SlaacConfig& orig);

    /// @brief Private assignment operator to avoid potential for slicing.
    ///
    /// @param rhs Context to be assigned.
    SlaacConfig& operator=(const SlaacConfig& rhs);

    /// Socket information will be stored here (for all supported servers)
    std::map<std::string, isc::data::ConstElementPtr> ctrl_sockets_;

    uint8_t hop_limit_;

    bool managed_flag_;

    bool other_flag_;

    uint16_t router_lifetime_;

    uint32_t reachable_time_;

    uint32_t retrans_timer_;

    bool src_ll_addr_;

    uint32_t mtu_;

    isc::data::ConstElementPtr universal_ra_;

    /// @brief Configured hooks libraries.
    isc::hooks::HooksConfig hooks_config_;
};

};
};


#endif