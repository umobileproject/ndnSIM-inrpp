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

#ifndef NFD_DAEMON_FW_INRPPFORWARDER_HPP
#define NFD_DAEMON_FW_INRPPFORWARDER_HPP

#include "forwarder.hpp"
#include "ns3/ndnSIM/model/cs/ndn-content-store.hpp"


namespace nfd {

namespace fw {
class Strategy;
} // namespace fw

typedef std::pair<Name,FaceId> nameFace;

class Face;
//typedef face::InrppState state;
/** \brief main class of NFD
 *
 *  Forwarder owns all faces and tables, and implements forwarding pipelines.
 */
class InrppForwarder : public Forwarder
{
public:
  InrppForwarder();

  VIRTUAL_WITH_TESTS
  ~InrppForwarder();

  int
  GetPackets(FaceId id);

  void
  sendData(FaceId id,uint64_t bps);

PUBLIC_WITH_TESTS_ELSE_PROTECTED: // pipelines
  /** \brief outgoing Data pipeline
   */
  VIRTUAL_WITH_TESTS void
  onOutgoingData(const Data& data, Face& inFace, Face& outFace);

  VIRTUAL_WITH_TESTS void
  onIncomingData(Face& inFace, const Data& data);

  /** \brief outgoing Interest pipeline
    */
   VIRTUAL_WITH_TESTS void
   onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry, Face& outFace, const Interest& interest);

private:
  void
  //onContentStoreHit( Face& outFace,const shared_ptr<pit::Entry>& pitEntry, const Interest& interest, const Data& data);
  onContentStoreHit(FaceId id, const Interest& interest, const Data& data);

  void
  //onContentStoreMiss( Face& inFace, const shared_ptr<pit::Entry>& pitEntry,const Interest& interest);
  onContentStoreMiss(FaceId id, const Interest& interest);
private:

  ns3::Ptr<ns3::ndn::ContentStore> m_csFromNdnSim;
  std::multimap<FaceId,nameFace> m_outTable;
  std::map<FaceId,uint32_t> m_bytes;
  std::map<FaceId,double> m_queueTime;

};

} // namespace nfd

#endif // NFD_DAEMON_FW_INRPPFORWARDER_HPP
