print("Loading hfudt")
--bit32 = require("bit32")

local control_types = {
  [0] = "ACK",
  [1] = "Handshake",
  [2] = "HandshakeACK",
  [3] = "HandshakeRequest"
}

local message_positions = {
  [0] = "ONLY",
  [1] = "LAST",
  [2] = "FIRST",
  [3] = "MIDDLE"
}

local packet_types = {
  [0] = "Unknown",
  [1] = "DomainConnectRequestPending",
  [2] = "DomainList",
  [3] = "Ping",
  [4] = "PingReply",
  [5] = "KillAvatar",
  [6] = "AvatarData",
  [7] = "InjectAudio",
  [8] = "MixedAudio",
  [9] = "MicrophoneAudioNoEcho",
  [10] = "MicrophoneAudioWithEcho",
  [11] = "BulkAvatarData",
  [12] = "SilentAudioFrame",
  [13] = "DomainListRequest",
  [14] = "RequestAssignment",
  [15] = "CreateAssignment",
  [16] = "DomainConnectionDenied",
  [17] = "MuteEnvironment",
  [18] = "AudioStreamStats",
  [19] = "DomainServerPathQuery",
  [20] = "DomainServerPathResponse",
  [21] = "DomainServerAddedNode",
  [22] = "ICEServerPeerInformation",
  [23] = "ICEServerQuery",
  [24] = "OctreeStats",
  [25] = "SetAvatarTraits",
  [26] = "AvatarIdentityRequest",
  [27] = "AssignmentClientStatus",
  [28] = "NoisyMute",
  [29] = "AvatarIdentity",
  [30] = "NodeIgnoreRequest",
  [31] = "DomainConnectRequest",
  [32] = "DomainServerRequireDTLS",
  [33] = "NodeJsonStats",
  [34] = "OctreeDataNack",
  [35] = "StopNode",
  [36] = "AudioEnvironment",
  [37] = "EntityEditNack",
  [38] = "ICEServerHeartbeat",
  [39] = "ICEPing",
  [40] = "ICEPingReply",
  [41] = "EntityData",
  [42] = "EntityQuery",
  [43] = "EntityAdd",
  [44] = "EntityErase",
  [45] = "EntityEdit",
  [46] = "DomainServerConnectionToken",
  [47] = "DomainSettingsRequest",
  [48] = "DomainSettings",
  [49] = "AssetGet",
  [50] = "AssetGetReply",
  [51] = "AssetUpload",
  [52] = "AssetUploadReply",
  [53] = "AssetGetInfo",
  [54] = "AssetGetInfoReply",
  [55] = "DomainDisconnectRequest",
  [56] = "DomainServerRemovedNode",
  [57] = "MessagesData",
  [58] = "MessagesSubscribe",
  [59] = "MessagesUnsubscribe",
  [60] = "ICEServerHeartbeatDenied",
  [61] = "AssetMappingOperation",
  [62] = "AssetMappingOperationReply",
  [63] = "ICEServerHeartbeatACK",
  [64] = "NegotiateAudioFormat",
  [65] = "SelectedAudioFormat",
  [66] = "MoreEntityShapes",
  [67] = "NodeKickRequest",
  [68] = "NodeMuteRequest",
  [69] = "RadiusIgnoreRequest",
  [70] = "UsernameFromIDRequest",
  [71] = "UsernameFromIDReply",
  [72] = "AvatarQuery",
  [73] = "RequestsDomainListData",
  [74] = "PerAvatarGainSet",
  [75] = "EntityScriptGetStatus",
  [76] = "EntityScriptGetStatusReply",
  [77] = "ReloadEntityServerScript",
  [78] = "EntityPhysics",
  [79] = "EntityServerScriptLog",
  [80] = "AdjustAvatarSorting",
  [81] = "OctreeFileReplacement",
  [82] = "CollisionEventChanges",
  [83] = "ReplicatedMicrophoneAudioNoEcho",
  [84] = "ReplicatedMicrophoneAudioWithEcho",
  [85] = "ReplicatedInjectAudio",
  [86] = "ReplicatedSilentAudioFrame",
  [87] = "ReplicatedAvatarIdentity",
  [88] = "ReplicatedKillAvatar",
  [89] = "ReplicatedBulkAvatarData",
  [90] = "DomainContentReplacementFromUrl",
  [91] = "ChallengeOwnership",
  [92] = "EntityScriptCallMethod",
  [93] = "ChallengeOwnershipRequest",
  [94] = "ChallengeOwnershipReply",
  [95] = "OctreeDataFileRequest",
  [96] = "OctreeDataFileReply",
  [97] = "OctreeDataPersist",
  [98] = "EntityClone",
  [99] = "EntityQueryInitialResultsComplete",
  [100] = "BulkAvatarTraits",
  [101] = "AudioSoloRequest",
  [102] = "BulkAvatarTraitsAck",
  [103] = "StopInjector",
  [104] = "AvatarZonePresence",
  [105] = "WebRTCSignaling"
}

-- PacketHeaders.h, getNonSourcedPackets()
-- all unsourced packets are also nonverified, see NLPacket::localHeaderSize
local unsourced_packet_types = {
  ["DomainConnectRequestPending"] = true,
  ["CreateAssignment"] = true,
  ["RequestAssignment"] = true,
  ["DomainServerRequireDTLS"] = true,
  ["DomainConnectRequest"] = true,
  ["DomainList"] = true,
  ["DomainConnectionDenied"] = true,
  ["DomainServerPathQuery"] = true,
  ["DomainServerPathResponse"] = true,
  ["DomainServerAddedNode"] = true,
  ["DomainServerConnectionToken"] = true,
  ["DomainSettingsRequest"] = true,
  ["OctreeDataFileRequest"] = true,
  ["OctreeDataFileReply"] = true,
  ["OctreeDataPersist"] = true,
  ["DomainContentReplacementFromUrl"] = true,
  ["DomainSettings"] = true,
  ["ICEServerPeerInformation"] = true,
  ["ICEServerQuery"] = true,
  ["ICEServerHeartbeat"] = true,
  ["ICEServerHeartbeatACK"] = true,
  ["ICEPing"] = true,
  ["ICEPingReply"] = true,
  ["ICEServerHeartbeatDenied"] = true,
  ["AssignmentClientStatus"] = true,
  ["StopNode"] = true,
  ["DomainServerRemovedNode"] = true,
  ["UsernameFromIDReply"] = true,
  ["OctreeFileReplacement"] = true,
  ["ReplicatedMicrophoneAudioNoEcho"] = true,
  ["ReplicatedMicrophoneAudioWithEcho"] = true,
  ["ReplicatedInjectAudio"] = true,
  ["ReplicatedSilentAudioFrame"] = true,
  ["ReplicatedAvatarIdentity"] = true,
  ["ReplicatedKillAvatar"] = true,
  ["ReplicatedBulkAvatarData"] = true,
  ["AvatarZonePresence"] = true,
  ["WebRTCSignaling"] = true
}

-- PacketHeaders.h, getNonVerifiedPackets()
local nonverified_packet_types = {
    ["NodeJsonStats"] = true,
    ["EntityQuery"] = true,
    ["OctreeDataNack"] = true,
    ["EntityEditNack"] = true,
    ["DomainListRequest"] = true,
    ["StopNode"] = true,
    ["DomainDisconnectRequest"] = true,
    ["UsernameFromIDRequest"] = true,
    ["NodeKickRequest"] = true,
    ["NodeMuteRequest"] = true,
}

local control_packet_types = {
  [0] = "Data",
  [1] = "Control"
}

local reliability_bit_types = {
  [0] = "Unreliable",
  [1] = "Reliable"
}

local message_bit_types = {
  [0] = "Single message",
  [1] = "Multipart message"
}

local message_position_types = {
  [0] = "Only",  -- 0x01
  [1] = "Last",  -- 0x01
  [2] = "First", -- 0x10
  [3] = "Middle" -- 0x11
}

-- create the HFUDT protocol
p_hfudt = Proto("hfudt", "HFUDT Protocol")

-- create fields shared between packets in HFUDT
local f_data = ProtoField.string("hfudt.data", "Data")

-- create the fields for data packets in HFUDT
local f_control_bit         = ProtoField.uint8 ("hfudt.control"            , "Control Bit"        , base.DEC, control_packet_types  , 0x80000000)
local f_reliable_bit        = ProtoField.uint8 ("hfudt.reliable"           , "Reliability Bit"    , base.DEC, reliability_bit_types , 0x40000000)
local f_message_bit         = ProtoField.uint8 ("hfudt.message"            , "Message Bit"        , base.DEC, message_bit_types     , 0x20000000)
local f_obfuscation_level   = ProtoField.uint8 ("hfudt.obfuscation_level"  , "Obfuscation Level"  , base.DEC, nil                   , 0x18000000)
local f_sequence_number     = ProtoField.uint32("hfudt.sequence_number"    , "Sequence Number"    , base.HEX, nil                   , 0x07FFFFFF)
local f_message_position    = ProtoField.uint8 ("hfudt.message_position"   , "Message Position"   , base.DEC, message_position_types, 0xC0000000)
local f_message_number      = ProtoField.uint32("hfudt.message_number"     , "Message Number"     , base.DEC, nil                   , 0x3FFFFFFF)
local f_message_part_number = ProtoField.uint32("hfudt.message_part_number", "Message Part Number", base.DEC)
local f_type = ProtoField.uint8("hfudt.type", "Type", base.DEC, packet_types)
local f_version = ProtoField.uint8("hfudt.version", "Version", base.DEC)
local f_sender_id = ProtoField.uint16("hfudt.sender_id", "Sender ID", base.DEC)
local f_hmac_hash = ProtoField.bytes("hfudt.hmac_hash", "HMAC Hash")

-- create the fields for control packets in HFUDT
local f_control_type        = ProtoField.uint32("hfudt.control_type"     , "Control Type"      , base.DEC, control_types, 0x7FFF0000)

-- control packets are formatted like data packets. The bit fields in data packets are not used, but
-- their space is still reserved. 
local f_control_reserved1   = ProtoField.uint32("hfudt.control_reserved1", "Control reserved 1", base.HEX, nil          , 0x0000FFFF)
local f_control_reserved2   = ProtoField.uint32("hfudt.control_reserved2", "Control reserved 2", base.HEX, nil          , 0xC0000000)

--local f_control_type_text = ProtoField.string("hfudt.control_type_text", "Control Type Text", base.ASCII)
local f_initial_sequence_number     = ProtoField.uint32("hfudt.initial_sequence_number", "Initial Sequence number"      , base.HEX, nil       , 0x07FFFFFF)
local f_ack_sequence_number = ProtoField.uint32("hfudt.ack_sequence_number", "ACKed Sequence Number", base.HEX, nil,                          0x07FFFFFF) 

local SEQUENCE_NUMBER_MASK = 0x07FFFFFF

p_hfudt.fields = {
  f_control_bit, f_reliable_bit, f_message_bit, f_sequence_number, f_type, f_version,
  f_sender_id, f_hmac_hash,
  f_message_position, f_message_number, f_message_part_number, f_obfuscation_level,
  f_control_type, f_control_reserved1, f_control_reserved2, f_initial_sequence_number, f_ack_sequence_number, f_data
}



local fragments = {}

local RFC_5389_MAGIC_COOKIE = 0x2112A442

function p_hfudt.dissector(buf, pinfo, tree)

  -- make sure this isn't a STUN packet - those don't follow HFUDT format
  if buf:len() >= 8 and buf(4, 4):uint() == RFC_5389_MAGIC_COOKIE then
    return 0
  end

  -- validate that the packet length is at least the minimum control packet size
  if buf:len() < 4 then return end

  -- create a subtree for HFUDT
  subtree = tree:add(p_hfudt, buf(0))

  -- pull out the entire first word
  local first_word = buf(0, 4):le_uint()

  -- pull out the control bit and add it to the subtree
  local control_bit = bit.rshift(first_word, 31)
  subtree:add_le(f_control_bit, buf(0,4))

  local data_length = 0

  if control_bit == 1 then
    -- dissect the control packet
    pinfo.cols.protocol = p_hfudt.name .. " Control"

    -- remove the control bit and shift to the right to get the type value
    local shifted_type = bit.rshift(bit.lshift(first_word, 1), 17)
    --local type = subtree:add(f_control_type, shifted_type)

    local type = subtree:add_le(f_control_type, buf(0,4))
    subtree:add_le(f_control_reserved1, buf(0,4))



    if shifted_type == 0 then -- ACK
      subtree:add_le(f_control_reserved2, buf(4,4))
      subtree:add_le(f_ack_sequence_number, buf(4,4))
    elseif shifted_type == 1 then -- Handshake
      subtree:add_le(f_control_reserved2, buf(4,4))
      subtree:add_le(f_initial_sequence_number, buf(4,4))
    elseif shifted_type == 2 then -- HandshakeACK
      subtree:add_le(f_control_reserved2, buf(4,4))
      subtree:add_le(f_initial_sequence_number, buf(4,4))
    else
      data_length = buf:len() - 4

      -- no sub-sequence number, just read the data
      subtree:add(f_data, buf(4, data_length))
    end

    local name = control_types[shifted_type]
    if name ~= nil then
      pinfo.cols.info:append(" [Ctl: " .. name .. "]")
    end
  else
    -- dissect the data packet
    pinfo.cols.protocol = p_hfudt.name

    -- set the reliability bit
    subtree:add_le(f_reliable_bit, buf(0,4))

    local message_bit = bit.band(0x01, bit.rshift(first_word, 29))

    -- set the message bit
    subtree:add_le(f_message_bit, buf(0,4))

    -- read the obfuscation level
    local obfuscation_bits = bit.band(0x03, bit.rshift(first_word, 27))
    subtree:add_le(f_obfuscation_level, buf(0,4))

    -- read the sequence number
    subtree:add_le(f_sequence_number, buf(0,4))


    local payload_offset = 4

    local message_number = 0
    local message_part_number = 0
    local message_position = 0

    -- if the message bit is set, handle the second word
    if message_bit == 1 then
      payload_offset = 12

      local second_word = buf(4, 4):le_uint()

      -- read message position from upper 2 bits
      message_position = bit.rshift(second_word, 30)
      subtree:add_le(f_message_position, buf(4,4))

      -- read message number from lower 30 bits
      message_number = bit.band(second_word, 0x3FFFFFFF)
      subtree:add_le(f_message_number, buf(4,4))


      -- read the message part number
      message_part_number = buf(8, 4):le_uint()
      subtree:add_le(f_message_part_number, buf(8,4))
    end

    if obfuscation_bits ~= 0 then
      local newbuf = deobfuscate(message_bit, buf, obfuscation_bits)
      buf = newbuf:tvb("Unobfuscated")
    end

    -- read the type
    local packet_type = buf(payload_offset, 1):le_uint()
    local ptype = subtree:add_le(f_type, buf(payload_offset, 1))
    local packet_type_text = packet_types[packet_type]

    if packet_type_text  ~= nil then
      pinfo.cols.info:append(" [" .. packet_type_text .. "]")
    end
    -- read the version
    subtree:add_le(f_version, buf(payload_offset + 1, 1))

    local i = payload_offset + 2


    if unsourced_packet_types[packet_type_text] == nil then
      -- read node local ID
      local sender_id = buf(payload_offset + 2, 2)
      subtree:add_le(f_sender_id, sender_id)
      i = i + 2
    end

    -- Unsourced packets are also nonverified, see NLPacket::localHeaderSize
    if nonverified_packet_types[packet_type_text] then --== nil and unsourced_packet_types[packet_type_text] == nil then
        -- read HMAC MD5 hash
        subtree:add(f_hmac_hash, buf(i, 16))
        i = i + 16
    end

    local payload_to_dissect = nil

    -- check if we have part of a message that we need to re-assemble
    -- before it can be dissected
    -- limit array indices to prevent lock-up with arbitrary data
    if message_bit == 1 and message_position ~= 0 and message_number < 100
      and message_part_number < 100 then

      if fragments[message_number] == nil then
        fragments[message_number] = {}
      end

      if fragments[message_number][message_part_number] == nil then
        fragments[message_number][message_part_number] = {}
      end

      -- set the properties for this fragment
      fragments[message_number][message_part_number] = {
        payload = buf(i):bytes()
      }

      -- if this is the last part, set our maximum part number
      if message_position == 1 then
        fragments[message_number].last_part_number = message_part_number
      end

      -- if we have the last part
      -- enumerate our parts for this message and see if everything is present
      if fragments[message_number].last_part_number ~= nil then
        local i = 0
        local has_all = true

        local finalMessage = ByteArray.new()
        local message_complete = true

        while i <= fragments[message_number].last_part_number do
          if fragments[message_number][i] ~= nil then
            finalMessage = finalMessage .. fragments[message_number][i].payload
          else
            -- missing this part, have to break until we have it
            message_complete = false
          end

          i = i + 1
        end

        if message_complete then
          debug("Message " .. message_number .. " is " .. finalMessage:len())
          payload_to_dissect = ByteArray.tvb(finalMessage, message_number)
        end
      end

    else
      payload_to_dissect = buf(i):tvb()
    end

    if payload_to_dissect ~= nil then
      -- Domain packets
      if packet_type_text == "DomainList" or
         packet_type_text == "DomainConnectRequest" or
         packet_type_text == "DomainListRequest"
      then
        Dissector.get("hf-domain"):call(payload_to_dissect, pinfo, tree)
      end

      -- AvatarData or BulkAvatarDataPacket
      if packet_type_text == "AvatarData" or
         packet_type_text == "BulkAvatarData" or
         packet_type_text == "BulkAvatarTraits" then
        Dissector.get("hf-avatar"):call(payload_to_dissect, pinfo, tree)
      end

      if packet_type_text == "EntityEdit" then
        Dissector.get("hf-entity"):call(payload_to_dissect, pinfo, tree)
      end

      if packet_types[packet_type] == "MicrophoneAudioNoEcho" or
         packet_types[packet_type] == "MicrophoneAudioWithEcho" or
         packet_types[packet_type] == "SilentAudioFrame" then
        Dissector.get("hf-audio"):call(payload_to_dissect, pinfo, tree)
      end
    end

  end

  -- return the size of the header
  return buf:len()

end

function p_hfudt.init()
  local udp_dissector_table = DissectorTable.get("udp.port")

  for port=1000, 65000 do
    udp_dissector_table:add(port, p_hfudt)
  end
end

function deobfuscate(message_bit, buf, level)
  local out = ByteArray.new()
  out:set_size(buf:len())
  if (level == 1) then
    key = ByteArray.new("6362726973736574")
  elseif level == 2 then
    key = ByteArray.new("7362697261726461")
  elseif level == 3 then
    key = ByteArray.new("72687566666d616e")
  else
    return
  end

  local start = 4
  if message_bit == 1 then
    local start = 12
  end

  local p = 0
  for i = start, buf:len() - 1 do
    out:set_index(i, bit.bxor(buf(i, 1):le_uint(), key:get_index(7 - (p % 8))) )
    p = p + 1
  end

  return out
end
