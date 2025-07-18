#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

add_executable(
  mythbackend
  autoexpire.cpp
  autoexpire.h
  backendcontext.cpp
  backendcontext.h
  backendhousekeeper.cpp
  backendhousekeeper.h
  encoderlink.cpp
  encoderlink.h
  filetransfer.cpp
  filetransfer.h
  httpconfig.cpp
  httpconfig.h
  httpstatus.cpp
  httpstatus.h
  internetContent.cpp
  internetContent.h
  mainserver.cpp
  mainserver.h
  mediaserver.cpp
  mediaserver.h
  mythbackend.cpp
  mythbackend_commandlineparser.cpp
  mythbackend_commandlineparser.h
  mythbackend_main_helpers.cpp
  mythbackend_main_helpers.h
  mythsettings.cpp
  mythsettings.h
  playbacksock.cpp
  playbacksock.h
  recordingextender.cpp
  recordingextender.h
  scheduler.cpp
  scheduler.h
  servicesv2/preformat.h
  servicesv2/v2artworkInfo.h
  servicesv2/v2artworkInfoList.h
  servicesv2/v2backendInfo.h
  servicesv2/v2backendStatus.h
  servicesv2/v2blurayInfo.h
  servicesv2/v2buildInfo.h
  servicesv2/v2capture.cpp
  servicesv2/v2capture.h
  servicesv2/v2captureCard.h
  servicesv2/v2captureCardList.h
  servicesv2/v2castMember.h
  servicesv2/v2castMemberList.h
  servicesv2/v2channel.cpp
  servicesv2/v2channel.h
  servicesv2/v2channelGroup.h
  servicesv2/v2channelGroupList.h
  servicesv2/v2channelInfoList.h
  servicesv2/v2channelRestore.h
  servicesv2/v2channelScan.h
  servicesv2/v2commMethod.h
  servicesv2/v2config.cpp
  servicesv2/v2config.h
  servicesv2/v2connectionInfo.h
  servicesv2/v2content.cpp
  servicesv2/v2content.h
  servicesv2/v2country.h
  servicesv2/v2countryList.h
  servicesv2/v2cutList.h
  servicesv2/v2cutting.h
  servicesv2/v2databaseInfo.h
  servicesv2/v2databaseStatus.h
  servicesv2/v2dvr.cpp
  servicesv2/v2dvr.h
  servicesv2/v2encoder.h
  servicesv2/v2encoderList.h
  servicesv2/v2envInfo.h
  servicesv2/v2freqtable.h
  servicesv2/v2frontend.h
  servicesv2/v2frontendList.h
  servicesv2/v2genre.h
  servicesv2/v2genreList.h
  servicesv2/v2grabber.h
  servicesv2/v2guide.cpp
  servicesv2/v2guide.h
  servicesv2/v2input.h
  servicesv2/v2inputList.h
  servicesv2/v2labelValue.h
  servicesv2/v2language.h
  servicesv2/v2languageList.h
  servicesv2/v2lineup.h
  servicesv2/v2logInfo.h
  servicesv2/v2logMessage.h
  servicesv2/v2logMessageList.h
  servicesv2/v2markup.h
  servicesv2/v2markupList.h
  servicesv2/v2music.cpp
  servicesv2/v2music.h
  servicesv2/v2musicMetadataInfo.h
  servicesv2/v2musicMetadataInfoList.h
  servicesv2/v2myth.cpp
  servicesv2/v2myth.h
  servicesv2/v2playGroup.h
  servicesv2/v2powerPriority.h
  servicesv2/v2programAndChannel.h
  servicesv2/v2programGuide.h
  servicesv2/v2programList.h
  servicesv2/v2recording.h
  servicesv2/v2recordingProfile.h
  servicesv2/v2recRule.h
  servicesv2/v2recRuleFilter.h
  servicesv2/v2recRuleFilterList.h
  servicesv2/v2recRuleList.h
  servicesv2/v2serviceUtil.cpp
  servicesv2/v2serviceUtil.h
  servicesv2/v2settingList.h
  servicesv2/v2status.cpp
  servicesv2/v2status.h
  servicesv2/v2storageGroupDir.h
  servicesv2/v2storageGroupDirList.h
  servicesv2/v2systemEventList.h
  servicesv2/v2timeZoneInfo.h
  servicesv2/v2titleInfo.h
  servicesv2/v2titleInfoList.h
  servicesv2/v2versionInfo.h
  servicesv2/v2video.cpp
  servicesv2/v2video.h
  servicesv2/v2videoLookupInfo.h
  servicesv2/v2videoLookupInfoList.h
  servicesv2/v2videoMetadataInfo.h
  servicesv2/v2videoMetadataInfoList.h
  servicesv2/v2videoMultiplex.h
  servicesv2/v2videoMultiplexList.h
  servicesv2/v2videoSource.h
  servicesv2/v2videoSourceList.h
  servicesv2/v2videoStreamInfo.h
  servicesv2/v2videoStreamInfoList.h
  servicesv2/v2wolInfo.h
  upnpcdsmusic.cpp
  upnpcdsmusic.h
  upnpcdstv.cpp
  upnpcdstv.h
  upnpcdsvideo.cpp
  upnpcdsvideo.h)

if(TARGET Qt${QT_VERSION_MAJOR}::DBus)
  target_sources(mythbackend PRIVATE platforms/mythnetworkdbus.h
                                     platforms/mythnetworkdbus.cpp)
endif()

target_include_directories(mythbackend PRIVATE .)

target_link_libraries(
  mythbackend
  PUBLIC myth
         mythtv
         mythbase
         mythmetadata
         mythprotoserver
         mythupnp
         $<TARGET_NAME_IF_EXISTS:PkgConfig::VALGRIND>
         $<TARGET_NAME_IF_EXISTS:Qt${QT_VERSION_MAJOR}::DBus>)

install(TARGETS mythbackend RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES devicemaster.xml deviceslave.xml MXML_scpd.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/mythtv)

install(FILES config_backend_general.xml config_backend_database.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/mythtv/backend-config)
