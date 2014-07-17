/*
 * CSyncDataDb_test.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: ranger.shi
 */

#include "CSyncDataDb.h"

#include <cstdio>
#include <map>
#include <vector>
#include <algorithm>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

struct assign_fixture
{
	assign_fixture()
	{
		SyncData::CSyncDataDb db;
		pathTemp = "./";
		pathTemp = pathTemp/strprintf("CSyncDataDb_test_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
		db.InitializeSyncDataDb(pathTemp);
	}
	~assign_fixture()
	{
		SyncData::CSyncDataDb db;
		db.CloseSyncDataDb();
		boost::filesystem::remove_all(pathTemp);
	}
	boost::filesystem::path pathTemp;
};

void CreateAndInsertData(int height, const SyncData::CSyncCheckPoint& point)
{
	SyncData::CSyncDataDb db;
	SyncData::CSyncData	syncData;
	CDataStream sstream(SER_NETWORK, PROTOCOL_VERSION);
	sstream<<point;
	syncData.m_vchMsg.assign(sstream.begin(), sstream.end());
	BOOST_CHECK(db.WriteCheckpoint(height, syncData));
}

BOOST_FIXTURE_TEST_SUITE(CSyncDataDb_test, assign_fixture)

BOOST_AUTO_TEST_CASE(LoadCheckpoint_test)
{
	SyncData::CSyncDataDb db;
	std::map<int, uint256> points;
	std::vector<unsigned char> data;
	for (int i = 1; i < 15; ++i)
	{
		SyncData::CSyncData	syncData;
		SyncData::CSyncCheckPoint point;
		data.push_back(i);
		point.m_height = i;
		point.m_hashCheckpoint = Hash(data.begin(), data.end());
		CreateAndInsertData(i, point);
		points[i] = point.m_hashCheckpoint;
	}
	std::map<int, uint256> dbPoints;
	BOOST_CHECK(db.LoadCheckPoint(dbPoints));
	BOOST_CHECK(points.size() == dbPoints.size());
	BOOST_CHECK(std::equal(points.begin(), points.end(), dbPoints.begin()));
}

BOOST_AUTO_TEST_CASE(WriteReadCheckpoint_test)
{
	SyncData::CSyncDataDb db;
	SyncData::CSyncData	syncData;
	SyncData::CSyncCheckPoint point1;
	int height = 20;
	std::string data = "12345678";
	point1.m_hashCheckpoint = Hash(data.begin(), data.end());
	CreateAndInsertData(height, point1);
	BOOST_CHECK(db.ReadCheckpoint(height, syncData));
	SyncData::CSyncCheckPoint point;
	point.SetData(syncData);
	BOOST_CHECK(point.m_hashCheckpoint != uint256(0));
	BOOST_CHECK(point1.m_hashCheckpoint == point.m_hashCheckpoint);
	BOOST_CHECK(!db.ReadCheckpoint(100, syncData));
}

BOOST_AUTO_TEST_SUITE_END()
