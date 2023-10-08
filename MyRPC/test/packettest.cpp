#include "../protocol/packet.hpp"
#include "unittestcore.h"

TEST_CASE(Packet_Alloc) {
  Protocol::Packet pkt;
  pkt.Alloc(100);
  ASSERT_EQ(pkt.UseLen(), 0);
  ASSERT_EQ(pkt.Len(), 100);
}

TEST_CASE(Packet_ReAlloc) {
  Protocol::Packet pkt;
  pkt.Alloc(100);
  ASSERT_EQ(pkt.UseLen(), 0);
  ASSERT_EQ(pkt.Len(), 100);
  pkt.ReAlloc(200);
  ASSERT_EQ(pkt.UseLen(), 0);
  ASSERT_EQ(pkt.Len(), 200);
}

TEST_CASE(Packet_Used) {
  Protocol::Packet pkt;
  pkt.Alloc(100);
  pkt.UpdateUseLen(10);
  pkt.UpdateParseLen(6);
  ASSERT_EQ(pkt.Len(), 90);
  ASSERT_EQ(pkt.UseLen(), 10);
  ASSERT_EQ(pkt.NeedParseLen(), 4);
  uint8_t* dataRaw = pkt.DataRaw();
  uint8_t* data = pkt.Data();
  uint8_t* dataParse = pkt.DataParse();
  ASSERT_EQ(dataRaw + 10, data);
  ASSERT_EQ(dataRaw + 6, dataParse);
  Protocol::Packet pkt2;
  pkt2.CopyFrom(pkt);
  ASSERT_EQ(pkt2.Len(), 90);
  ASSERT_EQ(pkt2.UseLen(), 10);
  ASSERT_EQ(pkt2.NeedParseLen(), 4);
  dataRaw = pkt2.DataRaw();
  data = pkt2.Data();
  dataParse = pkt2.DataParse();
  ASSERT_EQ(dataRaw + 10, data);
  ASSERT_EQ(dataRaw + 6, dataParse);
}