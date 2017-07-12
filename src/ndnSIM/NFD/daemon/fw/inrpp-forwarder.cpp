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

#include "inrpp-forwarder.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"
#include "strategy.hpp"
#include "table/cleanup.hpp"
#include <ndn-cxx/lp/tags.hpp>
#include "face/null-face.hpp"
#include <boost/random/uniform_int_distribution.hpp>

namespace nfd {

NFD_LOG_INIT("InrppForwarder");

InrppForwarder::InrppForwarder() : Forwarder()
{
	NFD_LOG_DEBUG(this);

}

InrppForwarder::~InrppForwarder() = default;

void
InrppForwarder::onOutgoingData(const Data& data, Face& outFace)
{
	  if (outFace.getId() == face::INVALID_FACEID) {
	    NFD_LOG_WARN("onOutgoingData face=invalid data=" << data.getName());
	    return;
	  }
	  NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() << " data=" << data.getName() << " scope= " << outFace.getScope());

	 // std::cout << this << " onOutgoingData face=" << outFace.getId() << " data=" << data.getName() << std::endl;
	  // /localhost scope control
	  bool isViolatingLocalhost = outFace.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
	                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
	  if (isViolatingLocalhost) {
	    NFD_LOG_DEBUG("onOutgoingData face=" << outFace.getId() <<
	                  " data=" << data.getName() << " violates /localhost");
	    // (drop)
	    return;
	  }

	  // TODO traffic manager

	  // send Data
	 // std::size_t found = data.getName().toUri().find("/prefix");
	  //Name localName("/localhost");
	 // if(localName.isPrefixOf(data.getName()))
	  if(outFace.getScope()==ndn::nfd::FACE_SCOPE_LOCAL)
	  {
		  NFD_LOG_DEBUG("Prefix outgoingdata face=" << outFace.getId() <<
		 	                  " data=" << data.getName());
		  outFace.sendData(data);
		  ++m_counters.nOutData;
	  } else {
		  //auto interest = make_shared<ndn::Interest>(data.getName());
		  NFD_LOG_DEBUG("Prefix outgoingdata face=" << outFace.getId() <<
		 	                  " data=" << data.getName() << " size=" <<   data.getContent().size());
		  m_outTable.insert(std::pair<FaceId,Name>(outFace.getId(),data.getName()));

		  std::map<FaceId,uint32_t>::iterator it;
		  it = m_bytes.find(outFace.getId());
		  if(it != m_bytes.end())
		  {
			  NFD_LOG_DEBUG("Bytes in the queue="<<it->second);
			  it->second+= static_cast<uint32_t>(data.getContent().size());
		  } else
		  {
			  m_bytes.insert(std::pair<FaceId,uint32_t>(outFace.getId(),static_cast<uint32_t>(data.getContent().size())));

		  }
		  //shared_ptr<Data> dataCopyWithoutTag = make_shared<Data>(data);
		  //dataCopyWithoutTag->removeTag<lp::HopCountTag>();
		  //NFD_LOG_DEBUG("NFD CACHE");
		  //m_cs.insert(*dataCopyWithoutTag);

	  }
	  //outFace.sendData(data);
	  //++m_counters.nOutData;
}

void
InrppForwarder::sendData(FaceId id,uint64_t bps)
{
    //NFD_LOG_DEBUG(this << " SendData face=" << id << " outTable " << m_outTable.size() << " cslimit="<< m_cs.getLimit() << " size="<<m_cs.size());
	std::map<FaceId,Name>::iterator it = m_outTable.find(id);

	NFD_LOG_DEBUG("outTable size=" << m_outTable.size());
	if(it!=m_outTable.end())
	{

		Name name = it->second;
		NFD_LOG_DEBUG("Send=" << name << " " << m_cs.getLimit() << " " << m_cs.size());

	    const ndn::Interest& interest(name);
		m_cs.find(interest,
		               bind(&InrppForwarder::onContentStoreHit, this,id, _1, _2),
		               bind(&InrppForwarder::onContentStoreMiss, this, _1));
		//m_cs.find(it->second);
		NFD_LOG_DEBUG("outTable size=" << m_outTable.size());
		m_outTable.erase(it);
		NFD_LOG_DEBUG("outTable size=" << m_outTable.size());

		//NFD_LOG_DEBUG("outTable size=" << m_outTable.size());


	} else {

	}

	std::map<FaceId,uint32_t>::iterator it2 = m_bytes.find(id);
	if(it2!=m_bytes.end())
	{
		if(it2->second>0)
		{
			NFD_LOG_DEBUG("outTable bytes time=" << it2->second << " " <<id);
			std::map<FaceId,double>::iterator it3 = m_queueTime.find(id);
			if(it3!=m_queueTime.end()){
				NFD_LOG_DEBUG("outTable queue time=" << it2->second << " " << bps << " "<< (double)it2->second*8/bps);
				it3->second = (double)it2->second*8/bps;
			}else{
				m_queueTime.insert(std::pair<FaceId,double>(id,(double)it2->second*8/bps));
			}
		}

	}

}

int
InrppForwarder::GetPackets(FaceId id)
{
	return m_outTable.count(id);
}

void
InrppForwarder::onContentStoreHit(FaceId id, const Interest& interest, const Data& data)
//InrppForwarder::onContentStoreHit( Face& outFace, const shared_ptr<pit::Entry>& pitEntry, const Interest& interest, const Data& data)
{
    NFD_LOG_DEBUG("onContentStoreHit face=" << id << " " << data.getName());
	 Face* outFace = m_faceTable.get(id);
	 outFace->sendData(data);
	 ++m_counters.nOutData;

	std::map<FaceId,uint32_t>::iterator it;
	it = m_bytes.find(id);
	if(it != m_bytes.end())
	{
		NFD_LOG_DEBUG("Bytes in the queue="<<it->second << " " << id);
		it->second-= static_cast<uint32_t>(data.getContent().size());
		NFD_LOG_DEBUG("Bytes in the queue="<<it->second << " " << id);

	}
}

void
InrppForwarder::onContentStoreMiss(const Interest& interest)
//InrppForwarder::onContentStoreMiss( Face& inFace, const shared_ptr<pit::Entry>& pitEntry,const Interest& interest)
{
    NFD_LOG_DEBUG("onContentStoreMiss");

}
} // namespace nfd
