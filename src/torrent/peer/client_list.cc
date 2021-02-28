// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <algorithm>
#include <functional>

#include "torrent/exceptions.h"
#include "torrent/hash_string.h"
#include "torrent/peer/client_list.h"
#include "torrent/utils/string_manip.h"

namespace torrent {

ClientList::ClientList() {
  insert(ClientInfo::TYPE_UNKNOWN, nullptr, nullptr, nullptr);

  // Move this to a seperate initialize function in libtorrent.

  // Sorted by popularity to optimize search. This list is heavily
  // biased by my own prejudices, and not at all based on facts.

  // Top-tier (most-popular) clients.
  insert_helper(ClientInfo::TYPE_AZUREUS, "lt", nullptr, nullptr, "libTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "qB", nullptr, nullptr, "qBittorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "UT", nullptr, nullptr, "uTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "TR", nullptr, nullptr, "Transmission");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "DE", nullptr, nullptr, "DelugeTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "AZ", nullptr, nullptr, "Vuze");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "UM", nullptr, nullptr, "uTorrent Mac");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LT", nullptr, nullptr, "libtorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BT", nullptr, nullptr, "Mainline");
  insert_helper(ClientInfo::TYPE_MAINLINE, "M", nullptr, nullptr, "Mainline");
  insert_helper(ClientInfo::TYPE_AZUREUS, "A2", nullptr, nullptr, "aria2");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BC", nullptr, nullptr, "BitComet");
  insert_helper(ClientInfo::TYPE_AZUREUS, "XL", nullptr, nullptr, "Xunlei");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SD", nullptr, nullptr, "Xunlei");

  // Other clients.
  insert_helper(ClientInfo::TYPE_AZUREUS, "7T", nullptr, nullptr, "aTorrent");
  insert_helper(ClientInfo::TYPE_COMPACT, "A", nullptr, nullptr, "ABC");
  insert_helper(ClientInfo::TYPE_AZUREUS, "A~", nullptr, nullptr, "Ares");
  insert_helper(ClientInfo::TYPE_AZUREUS, "AG", nullptr, nullptr, "Ares");
  insert_helper(ClientInfo::TYPE_AZUREUS, "AN", nullptr, nullptr, "Ares");
  insert_helper(ClientInfo::TYPE_AZUREUS,
                "AR",
                nullptr,
                nullptr,
                "Ares"); // Ares is more likely than ArcticTorrent
  insert_helper(ClientInfo::TYPE_AZUREUS, "AT", nullptr, nullptr, "Artemis");
  insert_helper(ClientInfo::TYPE_AZUREUS, "AV", nullptr, nullptr, "Avicora");
  insert_helper(ClientInfo::TYPE_AZUREUS, "AX", nullptr, nullptr, "BitPump");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BB", nullptr, nullptr, "BitBuddy");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "BE", nullptr, nullptr, "BitTorrent SDK");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BF", nullptr, nullptr, "BitFlu");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BG", nullptr, nullptr, "BTGetit");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BI", nullptr, nullptr, "BiglyBT");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "bk", nullptr, nullptr, "BitKitten (libtorrent)");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BM", nullptr, nullptr, "BitMagnet");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "BP", nullptr, nullptr, "BitTorrent Pro");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BR", nullptr, nullptr, "BitRocket");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BS", nullptr, nullptr, "BTSlave");
  insert_helper(ClientInfo::TYPE_AZUREUS, "BW", nullptr, nullptr, "BitWombat");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "BX", nullptr, nullptr, "Bittorrent X");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "CB", nullptr, nullptr, "Shareaza Plus");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "CD", nullptr, nullptr, "Enhanced CTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "cT", nullptr, nullptr, "CuteTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "CT", nullptr, nullptr, "CTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "DP", nullptr, nullptr, "Propogate Data Client");
  insert_helper(ClientInfo::TYPE_AZUREUS, "EB", nullptr, nullptr, "EBit");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "ES", nullptr, nullptr, "Electric Sheep");
  insert_helper(ClientInfo::TYPE_AZUREUS, "FC", nullptr, nullptr, "FileCroc");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "FD", nullptr, nullptr, "Free Download Manager");
  insert_helper(ClientInfo::TYPE_AZUREUS, "FG", nullptr, nullptr, "FlashGet");
  insert_helper(ClientInfo::TYPE_AZUREUS, "FL", nullptr, nullptr, "Flud");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "FT", nullptr, nullptr, "FoxTorrent/RedSwoosh");
  insert_helper(ClientInfo::TYPE_AZUREUS, "FW", nullptr, nullptr, "FrostWire");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "FX", nullptr, nullptr, "Freebox BitTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "GR", nullptr, nullptr, "GetRight");
  insert_helper(ClientInfo::TYPE_AZUREUS, "GS", nullptr, nullptr, "GSTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "GT", nullptr, nullptr, "go.torrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "HL", nullptr, nullptr, "Halite");
  insert_helper(ClientInfo::TYPE_AZUREUS, "HN", nullptr, nullptr, "Hydranode");
  insert_helper(ClientInfo::TYPE_AZUREUS, "IL", nullptr, nullptr, "iLivid");
  insert_helper(ClientInfo::TYPE_AZUREUS, "JS", nullptr, nullptr, "JSTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "JT", nullptr, nullptr, "jTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "jT", nullptr, nullptr, "jTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "KG", nullptr, nullptr, "KGet");
  insert_helper(ClientInfo::TYPE_AZUREUS, "KT", nullptr, nullptr, "KTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LC", nullptr, nullptr, "LeechCraft");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LH", nullptr, nullptr, "LH-ABC");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LK", nullptr, nullptr, "linkage");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LP", nullptr, nullptr, "Lphant");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "Lr", nullptr, nullptr, "LibreTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "LW", nullptr, nullptr, "LimeWire");
  insert_helper(ClientInfo::TYPE_AZUREUS, "MG", nullptr, nullptr, "MediaGet");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "MO", nullptr, nullptr, "MonoTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "MP", nullptr, nullptr, "MooPolice");
  insert_helper(ClientInfo::TYPE_AZUREUS, "MR", nullptr, nullptr, "Miro");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "MT", nullptr, nullptr, "MoonlightTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "NE", nullptr, nullptr, "BT Next Evolution");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "NX", nullptr, nullptr, "Net Transport");
  insert_helper(
    ClientInfo::TYPE_COMPACT, "O", nullptr, nullptr, "Osprey Permaseed");
  insert_helper(ClientInfo::TYPE_AZUREUS, "OS", nullptr, nullptr, "OneSwarm");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "OT", nullptr, nullptr, "OmegaTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "PC", nullptr, nullptr, "CacheLogic");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "PI", nullptr, nullptr, "PicoTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "PT", nullptr, nullptr, "Popcorn Time");
  insert_helper(ClientInfo::TYPE_AZUREUS, "PD", nullptr, nullptr, "Pando");
  insert_helper(ClientInfo::TYPE_AZUREUS, "pX", nullptr, nullptr, "pHoeniX");
  insert_helper(ClientInfo::TYPE_COMPACT, "Q", nullptr, nullptr, "BTQueue");
  insert_helper(ClientInfo::TYPE_AZUREUS, "QD", nullptr, nullptr, "qqdownload");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "QT", nullptr, nullptr, "Qt 4 Torrent");
  insert_helper(ClientInfo::TYPE_COMPACT, "R", nullptr, nullptr, "Tribler");
  insert_helper(ClientInfo::TYPE_AZUREUS, "RS", nullptr, nullptr, "Rufus");
  insert_helper(ClientInfo::TYPE_AZUREUS, "RT", nullptr, nullptr, "Retriever");
  insert_helper(ClientInfo::TYPE_AZUREUS, "RZ", nullptr, nullptr, "RezTorrent");
  insert_helper(
    ClientInfo::TYPE_COMPACT, "S", nullptr, nullptr, "Shadow's client");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "S~", nullptr, nullptr, "Shareaza alpha/beta");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SB", nullptr, nullptr, "SwiftBit");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SG", nullptr, nullptr, "GS Torrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SK", nullptr, nullptr, "Spark");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SM", nullptr, nullptr, "SoMud");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SN", nullptr, nullptr, "ShareNET");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SP", nullptr, nullptr, "BitSpirit");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SS", nullptr, nullptr, "SwarmScope");
  insert_helper(ClientInfo::TYPE_AZUREUS, "ST", nullptr, nullptr, "SymTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "st", nullptr, nullptr, "SharkTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "SZ", nullptr, nullptr, "Shareaza");
  insert_helper(ClientInfo::TYPE_AZUREUS, "tT", nullptr, nullptr, "tTorrent");
  insert_helper(ClientInfo::TYPE_COMPACT, "T", nullptr, nullptr, "BitTornado");
  insert_helper(ClientInfo::TYPE_AZUREUS, "TB", nullptr, nullptr, "Torch");
  insert_helper(ClientInfo::TYPE_AZUREUS, "TG", nullptr, nullptr, "Torrent GO");
  insert_helper(ClientInfo::TYPE_AZUREUS, "TL", nullptr, nullptr, "Tribler");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "TN", nullptr, nullptr, "Torrent.NET");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "TS", nullptr, nullptr, "Torrentstorm");
  insert_helper(ClientInfo::TYPE_AZUREUS, "TT", nullptr, nullptr, "TuoTu");
  insert_helper(
    ClientInfo::TYPE_COMPACT, "U", nullptr, nullptr, "UPnP NAT BitTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "UE", nullptr, nullptr, "uTorrent Embedded");
  insert_helper(ClientInfo::TYPE_AZUREUS, "UL", nullptr, nullptr, "uLeecher!");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "UW", nullptr, nullptr, "uTorrent Web");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "WD", nullptr, nullptr, "WebTorrent Desktop");
  insert_helper(ClientInfo::TYPE_AZUREUS, "WT", nullptr, nullptr, "Bitlet");
  insert_helper(ClientInfo::TYPE_AZUREUS, "WW", nullptr, nullptr, "WebTorrent");
  insert_helper(
    ClientInfo::TYPE_AZUREUS, "WY", nullptr, nullptr, "FireTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "VG", nullptr, nullptr, "Vagaa");
  insert_helper(ClientInfo::TYPE_AZUREUS, "XC", nullptr, nullptr, "XTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "XF", nullptr, nullptr, "Xfplay");
  insert_helper(ClientInfo::TYPE_AZUREUS, "XT", nullptr, nullptr, "XanTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "XX", nullptr, nullptr, "XTorrent");
  insert_helper(ClientInfo::TYPE_AZUREUS, "ZO", nullptr, nullptr, "Zona");
  insert_helper(ClientInfo::TYPE_AZUREUS, "ZT", nullptr, nullptr, "ZipTorrent");
}

ClientList::~ClientList() {
  for (auto& client : *this)
    delete client.info();
}

ClientList::iterator
ClientList::insert(ClientInfo::id_type type,
                   const char*         key,
                   const char*         version,
                   const char*         upperVersion) {
  if (type >= ClientInfo::TYPE_MAX_SIZE)
    throw input_error("Invalid client info id type.");

  ClientInfo clientInfo;

  clientInfo.set_type(type);
  clientInfo.set_info(new ClientInfo::info_type);
  clientInfo.set_short_description("Unknown");

  std::memset(clientInfo.mutable_key(), 0, ClientInfo::max_key_size);

  if (key == nullptr)
    std::memset(clientInfo.mutable_key(), 0, ClientInfo::max_key_size);
  else
    std::memcpy(clientInfo.mutable_key(), key, ClientInfo::max_key_size);

  if (version != nullptr)
    std::memcpy(
      clientInfo.mutable_version(), version, ClientInfo::max_version_size);
  else
    std::memset(clientInfo.mutable_version(), 0, ClientInfo::max_version_size);

  if (upperVersion != nullptr)
    std::memcpy(clientInfo.mutable_upper_version(),
                upperVersion,
                ClientInfo::max_version_size);
  else
    std::memset(
      clientInfo.mutable_upper_version(), -1, ClientInfo::max_version_size);

  return base_type::insert(end(), clientInfo);
}

ClientList::iterator
ClientList::insert_helper(ClientInfo::id_type type,
                          const char*         key,
                          const char*         version,
                          const char*         upperVersion,
                          const char*         shortDescription) {
  char newKey[ClientInfo::max_key_size];

  std::memset(newKey, 0, ClientInfo::max_key_size);
  std::memcpy(newKey, key, ClientInfo::key_size(type));

  iterator itr = insert(type, newKey, version, upperVersion);
  itr->set_short_description(shortDescription);

  return itr;
}

// Make this properly honor const-ness.
bool
ClientList::retrieve_id(ClientInfo* dest, const HashString& id) const {
  if (id[0] == '-' && id[7] == '-' && std::isalpha(id[1]) &&
      std::isalpha(id[2]) && std::isalnum(id[3]) && std::isalnum(id[4]) &&
      std::isalnum(id[5]) && std::isalnum(id[6])) {
    dest->set_type(ClientInfo::TYPE_AZUREUS);

    dest->mutable_key()[0] = id[1];
    dest->mutable_key()[1] = id[2];

    for (int i = 0; i < 4; i++)
      dest->mutable_version()[i] = dest->mutable_upper_version()[i] =
        utils::hexchar_to_value(id[3 + i]);

  } else if (std::isalpha(id[0]) && id[4] == '-' && std::isalnum(id[1]) &&
             std::isalnum(id[2]) && std::isalnum(id[3])) {
    dest->set_type(ClientInfo::TYPE_COMPACT);

    dest->mutable_key()[0] = id[0];
    dest->mutable_key()[1] = '\0';

    dest->mutable_version()[0] = dest->mutable_upper_version()[0] =
      utils::hexchar_to_value(id[1]);
    dest->mutable_version()[1] = dest->mutable_upper_version()[1] =
      utils::hexchar_to_value(id[2]);
    dest->mutable_version()[2] = dest->mutable_upper_version()[2] =
      utils::hexchar_to_value(id[3]);
    dest->mutable_version()[3] = dest->mutable_upper_version()[3] = '\0';

  } else if (std::isalpha(id[0]) && std::isdigit(id[1]) && id[2] == '-' &&
             std::isdigit(id[3]) && (id[6] == '-' || id[7] == '-')) {

    dest->set_type(ClientInfo::TYPE_MAINLINE);

    dest->mutable_key()[0] = id[0];
    dest->mutable_key()[1] = '\0';

    dest->mutable_version()[0] = dest->mutable_upper_version()[0] =
      utils::hexchar_to_value(id[1]);

    if (id[4] == '-' && std::isdigit(id[5]) && id[6] == '-') {
      dest->mutable_version()[1] = dest->mutable_upper_version()[1] =
        utils::hexchar_to_value(id[3]);
      dest->mutable_version()[2] = dest->mutable_upper_version()[2] =
        utils::hexchar_to_value(id[5]);
      dest->mutable_version()[3] = dest->mutable_upper_version()[3] = '\0';

    } else if (std::isdigit(id[4]) && id[5] == '-' && std::isdigit(id[6]) &&
               id[7] == '-') {
      dest->mutable_version()[1] = dest->mutable_upper_version()[1] =
        utils::hexchar_to_value(id[3]) * 10 + utils::hexchar_to_value(id[4]);
      dest->mutable_version()[2] = dest->mutable_upper_version()[2] =
        utils::hexchar_to_value(id[6]);
      dest->mutable_version()[3] = dest->mutable_upper_version()[3] = '\0';

    } else {
      *dest = *begin();
      std::memset(
        dest->mutable_upper_version(), 0, ClientInfo::max_version_size);

      return false;
    }

  } else {
    // And then the incompatible idiots that make life difficult for us
    // others. (There's '3' schemes to choose from already...)
    //
    // Or not...

    // The first entry always contains the default ClientInfo.
    *dest = *begin();
    std::memset(dest->mutable_upper_version(), 0, ClientInfo::max_version_size);

    return false;
  }

  const_iterator itr =
    std::find_if(begin() + 1, end(), [dest](const ClientInfo& info) {
      return ClientInfo::intersects(*dest, info);
    });

  if (itr == end())
    dest->set_info(begin()->info());
  else
    dest->set_info(itr->info());

  return true;
}

void
ClientList::retrieve_unknown(ClientInfo* dest) const {
  *dest = *begin();
  std::memset(dest->mutable_upper_version(), 0, ClientInfo::max_version_size);
}

} // namespace torrent
