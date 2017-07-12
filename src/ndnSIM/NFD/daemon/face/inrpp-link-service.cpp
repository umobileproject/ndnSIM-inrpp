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

#include "inrpp-link-service.hpp"
#include "generic-link-service.hpp"
#include "fw/inrpp-forwarder.hpp"
#include <ndn-cxx/lp/tags.hpp>

namespace nfd {
namespace face {

NFD_LOG_INIT("InrppLinkService");


InrppLinkService::InrppLinkService(const GenericLinkService::Options& options, shared_ptr<nfd::Forwarder> forwarder, uint64_t bps)
  : GenericLinkServiceCounters(m_reassembler), GenericLinkService(options)

{
	NFD_LOG_FACE_TRACE(this);
	m_forwarder = forwarder;
	double timing = (double)1500*8/bps*1000000000;
	m_adjustCapacityInterval = time::nanoseconds((int)timing);
	m_bps = bps;
	NFD_LOG_DEBUG("Time at " << timing);
	m_adjustCapacityEvent = scheduler::schedule(m_adjustCapacityInterval,bind(&InrppLinkService::PullPacketFromCS, this));
}
void
InrppLinkService::PullPacketFromCS()
{
	NFD_LOG_FACE_TRACE(this);
	auto f = dynamic_cast<InrppForwarder*>(m_forwarder.get());
	f->sendData(getFace()->getId(),m_bps);
	NFD_LOG_DEBUG("Table packets "<< f->GetPackets(getFace()->getId()) << " "<<m_adjustCapacityInterval);
	m_adjustCapacityEvent = scheduler::schedule(m_adjustCapacityInterval,bind(&InrppLinkService::PullPacketFromCS, this));

}



} // namespace face
} // namespace nfd
