-- create the domain protocol
print("Loading hf-domain")
print("Wireshark version " .. get_version())
bit32 = require("bit32")
p_hf_domain = Proto("hf-domain", "HF Domain Protocol")


local connect_reasons = {
  [0] = "Connect",
  [1] = "SilentDomainDisconnect",
  [2] = "Awake"
}

-- NoteType_t in NodeType.h, uint8
local node_types = {
  [68] = "DomainServer", -- D
  [111] = "EntityServer", -- o
  [73] = "Agent", -- I
  [77] = "AudioMixer", -- M
  [87] = "AvatarMixer", -- W
  [65] = "AssetServer", -- A
  [109] = "MessagesMixer", -- m
  [83] = "EntityScriptServer", -- S
  [66] = "UpstreamAudioMixer", -- B
  [67] = "UpstreamAvatarMixer", -- C
  [97] = "DownstreamAudioMixer", -- a
  [119] = "DownstreamAvatarMixer", -- w
  [0] = "Unknown (error)",
  [1] = "Unassigned"
}

-- SocketType.h, uint8
local socket_types = {
  [0] = "Unknown",
  [1] = "UDP",
  [2] = "WebRTC"
}

-- https://doc.qt.io/qt-5/qabstractsocket.html#NetworkLayerProtocol-enum
local qhostaddress_protocols = {
  [0] = "QAbstractSocket::IPv4Protocol",
  [1] = "QAbstractSocket::IPv6Protocol",
  [2] = "QAbstractSocket::AnyIPProtocol",
  [-1] = "QAbstractSocket::UnknownNetworkLayerProtocol"
}




-- domain packet fields
local f_domain_id = ProtoField.guid("hf_domain.domain_id", "Domain ID")
local f_domain_local_id = ProtoField.uint16("hf_domain.domain_local_id", "Domain Local ID")
local f_protocol_signature_len_id = ProtoField.uint32("hf_domain.protocol_signature_len", "Protocol Signature Length")
local f_protocol_signature_id = ProtoField.guid("hf_domain.protocol_signature", "Protocol Signature")
local f_macaddress_len_id = ProtoField.uint32("hf_domain.macaddress_len", "MAC Address Length")
local f_macaddress_id = ProtoField.string("hf_domain.macaddress", "MAC Address")

local f_fingerprint_len_id = ProtoField.uint32("hf_domain.fingerprint_len", "Machine Fingerprint Length")
local f_fingerprint_id = ProtoField.guid("hf_domain.fingerprint", "Machine Fingerprint")

local f_sysinfo_len_id = ProtoField.uint32("hf_domain.fingerprint_len", "System Info Length")
-- Uncompressed length seems to be a Qt addition, we don't write it intentionally.
local f_sysinfo_uncompressed_len_id = ProtoField.uint32("hf_domain.sysinfo_uncompressed_len", "System Info Uncompressed Length")
local f_sysinfo_id = ProtoField.string("hf_domain.sysinfo", "System Info")


local f_connect_reason_id = ProtoField.uint32("hf_domain.connect_reason", "Connect Reason", base.HEX, connect_reasons)
local f_uptime_id = ProtoField.uint64("hf_domain.uptime", "Uptime")
local f_time_id = ProtoField.uint64("hf_domain.time", "Time")

local f_owner_type_id = ProtoField.uint8("hf_domain.owner_type", "Owner type", base.HEX, node_types)

local f_public_sockaddr_type_id = ProtoField.uint8("hf_domain.public_sockaddr_type", "Public Address Type", base.hex, socket_types)
local f_public_sockaddr_qt_type_id = ProtoField.uint8("hf_domain.public_sockaddr_qt_type", "Public Address Qt Type", base.hex, qhostaddress_protocols)
local f_public_sockaddr_id = ProtoField.ipv4("hf_domain.public_sockaddr", "Public Address")
local f_public_port_id = ProtoField.uint16("hf_domain.public_port", "Public Port")

local f_local_sockaddr_type_id = ProtoField.uint8("hf_domain.local_sockaddr_type", "Local Address Type", base.hex, socket_types)
local f_local_sockaddr_qt_type_id = ProtoField.uint8("hf_domain.local_sockaddr_qt_type", "Local Address Qt Type", base.hex, qhostaddress_protocols)
local f_local_sockaddr_id = ProtoField.ipv4("hf_domain.local_sockaddr_type", "Local Address")
local f_local_port_id = ProtoField.uint16("hf_domain.local_port", "Local Port")

local f_nodes_of_interest_count_id = ProtoField.uint32("hf_domain.nodes_of_interest_count", "Number of Nodes of Interest")
local f_nodes_of_interest_id = ProtoField.string("hf_domain.nodes_of_interest", "Nodes of Interest")


local f_placename_length_id = ProtoField.uint32("hf_domain.placename_len", "Placename Length")
local f_placename_id = ProtoField.string("hf_domain.placename", "Placename")

local f_username_length_id = ProtoField.uint32("hf_domain.username_len", "Username Length")
local f_username_id = ProtoField.string("hf_domain.username", "Username")

local f_username_signature_length_id = ProtoField.uint32("hf_domain.username_signature_len", "Username Signature Length")
local f_username_signature_id = ProtoField.string("hf_domain.username_signature", "Username Signature")

local packet_type_extractor = Field.new('hfudt.type')
local packet_version_extractor = Field.new('hfudt.version')

local ef_version_unsupported = ProtoExpert.new("hfudt.version_unsupported.expert", "Protocol version unsupported by decoder", expert.group.UNDECODED, expert.severity.ERROR)
local ef_zlib_unsupported = ProtoExpert.new("hfudt.zlib_unsupported.expert", "zlib decompression not supported by this Wireshark version, 4.3.0 or later required.", expert.group.UNDECODED, expert.severity.WARN)


p_hf_domain.fields = {
  f_domain_id, f_domain_local_id,
  f_protocol_signature_len_id, f_protocol_signature_id,
  f_macaddress_len_id, f_macaddress_id,
  f_fingerprint_len_id, f_fingerprint_id,
  f_sysinfo_len_id,  f_sysinfo_uncompressed_len_id, f_sysinfo_id,
  f_connect_reason_id,
  f_uptime_id, f_time_id, f_owner_type_id,
  f_public_sockaddr_type_id, f_public_sockaddr_qt_type_id, f_public_sockaddr_id, f_public_port_id,
  f_local_sockaddr_type_id, f_local_sockaddr_qt_type_id, f_local_sockaddr_id, f_local_port_id,
  f_nodes_of_interest_count_id, f_nodes_of_interest_id,
  f_placename_length_id, f_placename_id,
  f_username_length_id, f_username_id,
  f_username_signature_length_id, f_username_signature_id
}

p_hf_domain.experts = { ef_version_unsupported, ef_zlib_unsupported }

function p_hf_domain.dissector(buf, pinfo, tree)
  pinfo.cols.protocol = p_hf_domain.name

  local packet_type = packet_type_extractor().value
  local packet_version = packet_version_extractor().value


  domain_subtree = tree:add(p_hf_domain, buf())


  if packet_type == 31 then
    -- DomainConnectRequest

    local i = 0
    local len = 0

    if packet_version == 27 then

      -- we're writing 32 bit sizes everywhere, see BasePacket.cpp
      len = buf(i, 4):uint()
      domain_subtree:add(f_protocol_signature_len_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_protocol_signature_id, buf(i, len)); i = i + len

      len = buf(i, 4):uint()
      domain_subtree:add(f_macaddress_len_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_macaddress_id, buf(i,len), buf(i, len):ustring()); i = i + len

      domain_subtree:add(f_fingerprint_id, buf(i, 16)); i = i + 16


      len = buf(i, 4):uint()
      domain_subtree:add(f_sysinfo_len_id, buf(i, 4)); i = i + 4
      --print("Sysinfo raw: " .. buf(i, len):uncompress_zlib("sysinfo"))
      --domain_subtree:add(f_sysinfo_id, buf(i, len):uncompress_zlib("sysinfo"))

      domain_subtree:add(f_sysinfo_uncompressed_len_id, buf(i, 4));

      if not pcall(function() domain_subtree:add(f_sysinfo_id, buf(i+4, len-4):uncompress_zlib("sysinfo")) end ) then
          -- we add +4 bytes and subtract -4 length to compensate for the 4 bytes uncompressed length Qt adds
          -- zlib decoding only supported starting with Wireshark 4.3
          domain_subtree:add(f_sysinfo_id, "[uncompressing zlib not supported in this Wireshark version]")
          tree:add_proto_expert_info(ef_zlib_unsupported)
      end
      i = i + len

      local reason = buf(i, 4):uint()
--      domain_subtree:add(f_connect_reason_id, string.format("%i [%s]", reason, connect_reasons[reason]))
      domain_subtree:add(f_connect_reason_id, buf(i, 4)); i = i + 4

      domain_subtree:add(f_uptime_id, buf(i, 8)); i = i + 8
      domain_subtree:add(f_time_id, buf(i, 8)); i = i + 8

      domain_subtree:add(f_owner_type_id, buf(i, 1)); i = i + 1

      domain_subtree:add(f_public_sockaddr_type_id, buf(i, 1)); i = i + 1
      domain_subtree:add(f_public_sockaddr_qt_type_id, buf(i, 1)); i = i + 1
      domain_subtree:add(f_public_sockaddr_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_public_port_id, buf(i, 2)); i = i + 2


      domain_subtree:add(f_local_sockaddr_type_id, buf(i, 1)); i = i + 1
      domain_subtree:add(f_local_sockaddr_qt_type_id, buf(i, 1)); i = i + 1
      domain_subtree:add(f_local_sockaddr_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_local_port_id, buf(i, 2)); i = i + 2

      -- TODO: This is an array of node_types, should be decoded as one
      local nodes_of_interest_count = buf(i, 4):uint()
      domain_subtree:add(f_nodes_of_interest_count_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_nodes_of_interest_id, buf(i, nodes_of_interest_count)); i = i + nodes_of_interest_count

      len = buf(i, 4):uint()
      domain_subtree:add(f_placename_length_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_placename_id, buf(i,len), buf(i, len):ustring()); i = i + len

      len = buf(i, 4):uint()
      domain_subtree:add(f_username_length_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_username_id, buf(i,len), buf(i, len):ustring()); i = i + len

      len = buf(i, 4):uint()
      domain_subtree:add(f_username_signature_length_id, buf(i, 4)); i = i + 4
      domain_subtree:add(f_username_signature_id, buf(i,len), buf(i, len):ustring()); i = i + len

    else
      tree:add_proto_expert_info(ef_version_unsupported)
    end
  else

    local i = 0

    domain_subtree:add(f_domain_id, buf(i, 16))
    i = i + 16

    domain_subtree:add_le(f_domain_local_id, buf(i, 2))
  end
end

