project(treasury)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( treasury treasury treasury.cpp )
target_include_directories( treasury PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( treasury ${CMAKE_SOURCE_DIR}/../ricardian )