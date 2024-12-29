macro(TARGET_DISCORD_RPC)
    find_package(discord-rpc REQUIRED)
    target_link_libraries(${TARGET_NAME} discord-rpc::discord-rpc)
endmacro()
