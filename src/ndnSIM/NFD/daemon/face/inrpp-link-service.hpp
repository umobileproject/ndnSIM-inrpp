/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_FACE_INRPP_LINK_SERVICE_HPP
#define NFD_DAEMON_FACE_INRPP_LINK_SERVICE_HPP

#include "generic-link-service.hpp"
#include "fw/forwarder.hpp"
#include "core/scheduler.hpp"

namespace nfd {
namespace face {


/** \brief GenericLinkService is a LinkService that implements the NDNLPv2 protocol
 *  \sa http://redmine.named-data.net/projects/nfd/wiki/NDNLPv2
 */
class InrppLinkService : public GenericLinkService
{
public:

  InrppLinkService(const GenericLinkService::Options& options,  shared_ptr<nfd::Forwarder>, uint64_t bps);

private:
  void
  PullPacketFromCS();

  void
  receiveInterest(const Interest& interest);

  void
  CancelClosedLoop();

  shared_ptr<nfd::Forwarder> m_forwarder;
  time::nanoseconds m_adjustCapacityInterval;
  scheduler::EventId m_adjustCapacityEvent, m_faceStateEvent;
  uint64_t m_bps;
};

} // namespace face
} // namespace nfd

#endif // NFD_DAEMON_FACE_INRPP_LINK_SERVICE_HPP
