set(TNL_SOURCES
	assert.cpp
	asymmetricKey.cpp
	bitStream.cpp
	byteBuffer.cpp
	certificate.cpp
	clientPuzzle.cpp
	connectionStringTable.cpp
	dataChunker.cpp
	eventConnection.cpp
	ghostConnection.cpp
	huffmanStringProcessor.cpp
	log.cpp
	netBase.cpp
	netConnection.cpp
	netInterface.cpp
	netObject.cpp
	netStringTable.cpp
	platform.cpp
	random.cpp
	rpc.cpp
	symmetricCipher.cpp
	thread.cpp
	tnlMethodDispatch.cpp
	journal.cpp
	udp.cpp
	vector.cpp
)

file(GLOB TNL_HEADERS "*.h")

include_directories(${TOMCRYPT_INCLUDE_DIR} SYSTEM)
add_library(tnl ${TNL_SOURCES} ${TNL_HEADERS})
target_link_libraries(tnl tomcrypt ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(tnl PROPERTIES COMPILE_DEFINITIONS_DEBUG "TNL_DEBUG;TNL_ENABLE_LOGGING")
