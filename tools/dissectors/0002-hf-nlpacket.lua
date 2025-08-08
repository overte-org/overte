print("Loading hf-nlpacket")



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

-- Mapping of packet type to decoder
local dissectors = {
  ["DomainList"] = "hf-domain",
  ["DomainConnectRequest"] = "hf-domain",
  ["DomainListRequest"] = "hf-domain",
  ["AvatarData"] = "hf-avatar",
  ["BulkAvatarData"] = "hf-avatar",
  ["BulkAvatarTraits"] = "hf-avatar",
  ["EntityEdit"] = "hf-entity",
  ["MicrophoneAudioNoEcho"] = "hf-audio",
  ["MicrophoneAudioWithEcho"] = "hf-audio",
  ["SilentAudioFrame"] = "hf-audio"
}


p_nlpacket = Proto("hf-nlpacket", "HF NLPacket")

local f_type = ProtoField.uint8("hf-nlpacket.type"           , "Type"     , base.DEC, packet_types)
local f_version = ProtoField.uint8("hf-nlpacket.version"     , "Version"  , base.DEC)
local f_sender_id = ProtoField.uint16("hf-nlpacket.sender_id", "Sender ID", base.DEC)
local f_hmac_hash = ProtoField.bytes("hf-nlpacket.hmac_hash" , "HMAC Hash")

-- We don't know what this is at all.
local ef_unknown_type = ProtoExpert.new("nf-nlpacket.unknown_type.expert", "Packet type unrecognized. Wireshark decoder too old?", expert.group.UNDECODED, expert.severity.ERROR)

-- We know what it is, but there's no decoder for it.
local ef_no_decoder = ProtoExpert.new("hf-nlpacket.no_decoder.expert", "Decoder for this packet hasn't been implemented yet", expert.group.UNDECODED, expert.severity.ERROR)

p_nlpacket.fields = {
  f_type, f_version, f_sender_id, f_hmac_hash
}

p_nlpacket.experts = { 
  ef_unknown_type, ef_no_decoder 
}

function p_nlpacket.dissector(buf, pinfo, tree)
  pinfo.cols.protocol = p_nlpacket.name

  local pos = 0

  -- Packet must at least have type and version
  if buf:len() < 2 then return end


  subtree = tree:add(p_nlpacket, buf(0))


  local packet_type = buf(0, 1):le_uint()
  local packet_type_text = packet_types[packet_type]

  

  subtree:add(f_type, buf(pos,1)); pos = pos + 1
  subtree:add(f_version, buf(pos,1)); pos = pos + 1


  if unsourced_packet_types[packet_type_text] == nil then
    subtree:add_le(f_sender_id, buf(pos,2))
    pos = pos + 2
  end

  if nonverified_packet_types[packet_type_text] == nil then --== nil and unsourced_packet_types[packet_type_text] == nil then
      -- read HMAC MD5 hash
      subtree:add(f_hmac_hash, buf(pos, 16))
      pos = pos + 16
  end

  ---------------------------------------------------------------------------
  -- Payload dissection
  ---------------------------------------------------------------------------

  if packet_type_text == nil then
    -- We don't even know what this packet type is. Please contribute!
    tree:add_proto_expert_info(ef_no_decoder)
    print("Packet type " .. packet_type .. " is not known")
    return
  end

  pinfo.cols.info:append(" [" .. packet_type_text .. "]")

  local payload_to_dissect = buf(pos):tvb()

  dissector = dissectors[packet_type_text]

  if dissector ~= nil then
    Dissector.get(dissector):call(payload_to_dissect, pinfo, tree)
  else
    -- We don't know how to decode this. Please contribute!
    tree:add_proto_expert_info(ef_no_decoder)
  end

end

