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

#include "cs-table.hpp"
#include "forwarder.hpp"
#include "core/global-io.hpp"
#include "core/logger.hpp"
#include "face/channel.hpp"

namespace nfd {

NFD_LOG_INIT("CsTable");

CsTable::CsTable()
  : m_lastFaceId(face::FACEID_RESERVED_MAX)
{
}

Cs*
CsTable::get(FaceId id) const
{
  auto i = m_cs.find(id);
  if (i == m_cs.end()) {
    return nullptr;
  }
  return i->second.get();
}

size_t
CsTable::size() const
{
  return m_cs.size();
}

void
CsTable::add(shared_ptr<Cs> cs,FaceId faceId)
{

  //FaceId faceId = ++m_lastFaceId;
  BOOST_ASSERT(faceId > face::FACEID_RESERVED_MAX);
  this->addImpl(cs, faceId);
}


void
CsTable::addImpl(shared_ptr<Cs> cs, FaceId faceId)
{
  //face->setId(faceId);
  m_cs[faceId] = cs;
  NFD_LOG_INFO("Added face id=" << faceId);// << " remote=" << face->getRemoteUri()
                                        //  << " local=" << face->getLocalUri());

 // connectFaceClosedSignal(*face, bind(&FaceTable::remove, this, faceId));

  //this->afterAdd(*face);
}

void
CsTable::remove(FaceId faceId)
{
  auto i = m_cs.find(faceId);
  BOOST_ASSERT(i != m_cs.end());
  shared_ptr<Cs> cs = i->second;

  //this->beforeRemove(*face);

  m_cs.erase(i);
  //face->setId(face::INVALID_FACEID);

  NFD_LOG_INFO("Removed face id=" << faceId );//<<
            //   " remote=" << face->getRemoteUri() <<
             //  " local=" << face->getLocalUri());

  // defer Face deallocation, so that Transport isn't deallocated during afterStateChange signal
  //getGlobalIoService().post([face] {});
}

CsTable::ForwardRange
CsTable::getForwardRange() const
{
  return m_cs | boost::adaptors::map_values | boost::adaptors::indirected;
}

CsTable::const_iterator
CsTable::begin() const
{
  return this->getForwardRange().begin();
}

CsTable::const_iterator
CsTable::end() const
{
  return this->getForwardRange().end();
}

} // namespace nfd
