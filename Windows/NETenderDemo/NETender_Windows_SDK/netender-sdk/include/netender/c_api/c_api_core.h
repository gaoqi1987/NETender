/**
 * @file c_api_core.h
 * @brief NETender C API Core Definitions
 * @version 2.0.0
 * @date 2025-10-18
 * 
 * This file contains core definitions for NETender C API:
 * - Export macros
 * - Version information
 * - Error codes
 */

#ifndef NETENDER_C_API_CORE_H_
#define NETENDER_C_API_CORE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================== Export Macros ====================

#if defined(_WIN32) || defined(_WIN64)
    #ifdef NETENDER_EXPORTS
        #define NETENDER_API __declspec(dllexport)
    #else
        #define NETENDER_API __declspec(dllimport)
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define NETENDER_API __attribute__((visibility("default")))
    #else
        #define NETENDER_API
    #endif
#endif

// ==================== Opaque Handle Types ====================

/**
 * @brief SDK handle (opaque pointer shared across managers)
 */
typedef struct NETenderSDK_T* NETenderSDKHandle;

/**
 * @brief MeetingKit handle (main SDK entry point)
 */
typedef struct NETenderMeetingKit_T* NETenderMeetingKitHandle;

/**
 * @brief Meeting manager handle (opaque pointer)
 */
typedef struct NETenderMeetingManager_T* NETenderMeetingManagerHandle;

/**
 * @brief Audio manager handle
 */
typedef struct NETenderAudioManager_T* NETenderAudioManagerHandle;

/**
 * @brief Video manager handle
 */
typedef struct NETenderVideoManager_T* NETenderVideoManagerHandle;

/**
 * @brief Member manager handle
 */
typedef struct NETenderMemberManager_T* NETenderMemberManagerHandle;

/**
 * @brief Security manager handle
 */
typedef struct NETenderSecurityManager_T* NETenderSecurityManagerHandle;

/**
 * @brief Waiting room manager handle
 */
typedef struct NETenderWaitingRoomManager_T* NETenderWaitingRoomManagerHandle;

/**
 * @brief Pre-meeting manager handle
 */
typedef struct NETenderPreMeetingManager_T* NETenderPreMeetingManagerHandle;

/**
 * @brief Invite manager handle
 */
typedef struct NETenderInviteManager_T* NETenderInviteManagerHandle;

/**
 * @brief Account manager handle
 */
typedef struct NETenderAccountManager_T* NETenderAccountManagerHandle;

/**
 * @brief Chatroom manager handle
 */
typedef struct NETenderChatroomManager_T* NETenderChatroomManagerHandle;

/**
 * @brief Screen share manager handle
 */
typedef struct NETenderScreenShareManager_T* NETenderScreenShareManagerHandle;

/**
 * @brief Meeting listener handle (opaque pointer)
 */
typedef struct NETenderMeetingListener_T* NETenderMeetingListenerHandle;

/**
 * @brief RTC stats listener handle (opaque pointer)
 */
typedef struct NETenderRtcStatsListener_T* NETenderRtcStatsListenerHandle;

/**
 * @brief Message channel listener handle (opaque pointer)
 */
typedef struct NETenderMessageChannelListener_T* NETenderMessageChannelListenerHandle;

// ==================== Version Information ====================

#define NETENDER_VERSION_MAJOR 1
#define NETENDER_VERSION_MINOR 1
#define NETENDER_VERSION_PATCH 1

/**
 * @brief Get SDK version string
 * @return Version string (e.g., "1.1.1")
 */
NETENDER_API const char* NETender_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif // NETENDER_C_API_CORE_H_
