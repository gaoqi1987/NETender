#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include "videowidget.h"

// SDK头文件
#include <netender/c_api/c_api_account_manager.h>
#include <netender/c_api/c_api_audio_manager.h>
#include <netender/c_api/c_api_chatroom_manager.h>
#include <netender/c_api/c_api_invite_manager.h>
#include <netender/c_api/c_api_meeting_kit.h>
#include <netender/c_api/c_api_meeting_manager.h>
#include <netender/c_api/c_api_member_manager.h>
#include <netender/c_api/c_api_pre_meeting_manager.h>
#include <netender/c_api/c_api_screen_share_manager.h>
#include <netender/c_api/c_api_security_manager.h>
#include <netender/c_api/c_api_video_manager.h>
#include <netender/c_api/c_api_waiting_room_manager.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

private slots:
    // SDK初始化
    void initializeSDK();
    void uninitializeSDK();

    // 账户管理
    void loginByPassword();
    void loginByToken();
    void logout();
    void getAccountInfo();
    void updateNickname();

    // 会议管理
    void createMeeting();
    void joinMeeting();
    void leaveMeeting();
    void endMeeting();
    void getMeetingInfo();

    // 音频管理
    void refreshSpeakerDevices();
    void refreshMicrophoneDevices();
    void setSpeakerDevice();
    void setMicrophoneDevice();
    void setSpeakerVolume();
    void setMicrophoneVolume();
    void muteMyAudio();
    void unmuteMyAudio();
    void startSpeakerTest();
    void stopSpeakerTest();
    void startMicrophoneTest();
    void stopMicrophoneTest();

    // 音频效果控制
    void setAudioEffect();

    // 视频管理
    void refreshVideoDevices();
    void setCameraDevice();
    void startVideoPreview();
    void stopVideoPreview();
    void muteMyVideo();
    void unmuteMyVideo();
    void setupLocalVideoCanvas();
    void removeLocalVideoCanvas();
    void setupVideoPreviewCanvas();
    void setupRemoteVideoCanvas(const QString& userId);
    void removeRemoteVideoCanvas(const QString& userId);
    void setupRemoteVideoSubStreamCanvas(const QString& userId);
    void removeRemoteVideoSubStreamCanvas(const QString& userId);

    // 视频配置和控制
    void setVideoProfile();
    void unmuteMyVideoEx();
    void muteMyVideoEx();

    // 远程视频流订阅控制
    void subscribeRemoteVideoStream();
    void unsubscribeRemoteVideoStream();
    void subscribeRemoteVideoSubStream();
    void unsubscribeRemoteVideoSubStream();

    // 屏幕共享
    void refreshScreenSources();
    void startScreenShare();
    void stopScreenShare();

    // 聊天室
    void sendChatMessage();

    // 成员管理
    void refreshMemberList();
    void muteMemberAudio();
    void unmuteMemberAudio();
    void muteAllAudio();

    // 邀请管理
    void searchAccountByPhone();
    void getAccountInfoList();

    // 预会议管理
    void getMeetingList();
    void scheduleMeeting();
    void editMeeting();
    void cancelMeeting();
    void getScheduledMembers();
    void getPreMeetingInfo();

    // 安全管理
    void getChatroomPermission();
    void changeChatroomPermission();

    // 等候室管理
    void getWaitingRoomMemberList();
    void getWaitingRoomInfo();
    void putInWaitingRoom();
    void admitMember();
    void expelMember();
    void admitAll();
    void expelAll();

    // 日志输出
    void logMessage(const QString& message);
    void clearLog();
    void saveLog();
    void toggleAutoScroll(bool enabled);
    void searchLog(const QString& searchText);

private:
    void setupUI();
    void setupSDKCallbacks();
    void unregisterSDKCallbacks();

    // UI组件
    QTabWidget* mainTabWidget;

    // 添加分割条和右侧区域控件
    QSplitter* mainSplitter;
    QTextEdit* logTextEdit;
    QPushButton* clearLogButton;
    QPushButton* saveLogButton;
    QPushButton* autoScrollButton;
    bool autoScrollEnabled;

    // SDK初始化控件
    QLineEdit* appKeyEdit;
    QLineEdit* serverUrlEdit;
    QComboBox* localeCombo;

    // 账户管理控件
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* accountIdEdit;
    QLineEdit* tokenEdit;

    // 会议管理控件
    QLineEdit* meetingSubjectEdit;
    QCheckBox* waitingRoomCheck;
    QCheckBox* recordingCheck;
    QLineEdit* meetingNumEdit;
    QLineEdit* joinNicknameEdit;
    QLineEdit* joinUserUuidEdit;

    QLineEdit* meetingPasswordEdit;
    QLineEdit* nickNameEdit;
    QCheckBox* noChatCheck;
    QCheckBox* noWhiteboardCheck;
    QCheckBox* noCaptionsCheck;
    QLineEdit* orgNameEdit;
    QLineEdit* avatarUrlEdit;
    QSpinBox* joinTimeoutSpin;
    QLineEdit* sessionEdit;
    QComboBox* meetingSubTypeCombo;

    // 加入会议新增控件
    QLineEdit* joinPasswordEdit;
    QLineEdit* joinnickNameEdit;
    QLineEdit* joinOrgNameEdit;
    QLineEdit* joinAvatarUrlEdit;
    QLineEdit* joinSessionEdit;

    // 音频管理控件
    QComboBox* speakerCombo;
    QComboBox* microphoneCombo;
    QSlider* speakerSlider;
    QSlider* micSlider;

    // 音频效果控件
    QComboBox* audioEffectTypeCombo;
    QSpinBox* audioEffectValueSpin;
    QCheckBox* enableAIDenoiseCheck;

    // 视频管理控件
    QComboBox* cameraCombo;
    QLabel* videoPreviewLabel;
    VideoRenderWidget* localVideoWidget;
    VideoRenderWidget* remoteVideoWidget;
    VideoRenderWidget* subStreamVideoWidget;
    VideoRenderWidget* previewVideoWidget;

    // 视频配置控件
    QComboBox* videoResolutionCombo;
    QSpinBox* frameRateSpinBox;

    // 远程视频订阅控件
    QLineEdit* subscribeUserIdEdit;
    QLineEdit* unsubscribeUserIdEdit;
    QLineEdit* subscribeSubStreamUserIdEdit;
    QLineEdit* unsubscribeSubStreamUserIdEdit;
    QComboBox* streamTypeCombo;

    // 屏幕共享控件
    QListWidget* screenSourceList;
    QCheckBox* systemAudioCheck;

    // 成员管理控件
    QTableWidget* memberTable;

    // 聊天室控件
    QTextEdit* chatHistory;
    QLineEdit* messageEdit;

    // 邀请管理控件
    QLineEdit* searchPhoneEdit;
    QLineEdit* userUuidsEdit;
    QListWidget* searchResultsList;

    // 预会议管理控件
    QLineEdit* scheduleSubjectEdit;
    QDateTimeEdit* scheduleStartTimeEdit;
    QDateTimeEdit* scheduleEndTimeEdit;
    QCheckBox* scheduleWaitingRoomCheck;
    QCheckBox* scheduleRecordingCheck;
    QLineEdit* editMeetingIdEdit;
    QLineEdit* cancelMeetingIdEdit;
    QLineEdit* getMeetingNumEdit;
    QTableWidget* meetingListTable;
    QLineEdit* editMeetingSubjectEdit;
    QDateTimeEdit* editMeetingStartTimeEdit;
    QDateTimeEdit* editMeetingEndTimeEdit;
    QCheckBox* editMeetingWaitingRoomCheck;
    QCheckBox* editMeetingRecordingCheck;
    QLineEdit* editMeetingMembersEdit;

    // 安全管理控件
    QComboBox* chatroomPermCombo;
    QCheckBox* waitingRoomChatCheck;
    QTextEdit* securityInfoEdit;

    // 等候室管理控件
    QLineEdit* waitingRoomUserUuidEdit;
    QTableWidget* waitingRoomMemberTable;
    QTextEdit* waitingRoomInfoEdit;

    // SDK状态
    bool sdkInitialized;
    NETenderSDKHandle sdkHandle;
    NETenderMeetingManagerHandle meetingManager;
    NETenderAccountManagerHandle accountManager;
    NETenderAudioManagerHandle audioManager;
    NETenderVideoManagerHandle videoManager;
    NETenderMemberManagerHandle memberManager;
    NETenderScreenShareManagerHandle screenShareManager;
    NETenderChatroomManagerHandle chatroomManager;
    NETenderInviteManagerHandle inviteManager;
    NETenderPreMeetingManagerHandle preMeetingManager;
    NETenderSecurityManagerHandle securityManager;
    NETenderWaitingRoomManagerHandle waitingRoomManager;

    NETenderGlobalEventListener globalEventListener;
    NETenderMeetingListener meetingListener;
    NETenderMessageChannelListener messageChannelListener;
    NETenderWaitingRoomListener waitingRoomListener;
    bool globalEventListenerRegistered;
    bool meetingListenerRegistered;
    bool messageChannelListenerRegistered;
    bool waitingRoomListenerRegistered;

    // 初始化选项卡
    QWidget* setupInitTab();
    QWidget* setupAccountTab();
    QWidget* setupMeetingTab();
    QWidget* setupAudioTab();
    QWidget* setupVideoTab();
    QWidget* setupScreenShareTab();
    QWidget* setupMemberTab();
    QWidget* setupChatTab();
    QWidget* setupInviteTab();
    QWidget* setupPreMeetingTab();
    QWidget* setupSecurityTab();
    QWidget* setupWaitingRoomTab();

    // 通用回调处理
    static void asyncCallback(int32_t code, const char* message, void* user_data);
    static void accountInfoCallback(int32_t code, const char* message, NETenderAccountInfo* account_info, void* user_data);
    static void deviceListCallback(int32_t code, const char* message, NETenderDeviceInfo* devices, uint32_t count, void* user_data);
    static void screenSourceCallback(int32_t code, const char* message, NETenderScreenCaptureSource* sources, uint32_t count, void* user_data);
    static void chatMessageCallback(int32_t code, const char* message, NETenderChatMessage* chat_message, void* user_data);
    static void accountListCallback(int32_t code, const char* message, NETenderSearchAccountInfo* accounts, int32_t count, void* user_data);
    static void meetingListCallback(int32_t code, const char* message, NETenderMeetingInfo* meetings, int32_t count, void* user_data);
    static void meetingInfoCallback(int32_t code, const char* message, NETenderMeetingInfo* meeting_info, void* user_data);
    static void scheduledMemberCallback(int32_t code, const char* message, NETenderScheduledMember* members, int32_t count, void* user_data);
    static void chatroomPermissionCallback(int32_t code, const char* message, NETenderChatroomPermission* permission, void* user_data);
    static void waitingRoomMemberCallback(int32_t code, const char* message, NETenderWaitingRoomMember* members, int32_t count, void* user_data);

    static void postListenerLog(void* user_data, const QString& message);
    static QString safeText(const char* text, const QString& fallback = QStringLiteral("未知"));
    static QString boolStateText(int32_t enabled);
    static QString memberSummary(const NETenderMemberInfo* member);
    static QString chatMessageSummary(const NETenderRoomChatMessage* message);

    static void onBeforeRtcEngineInitialize(const char* room_uuid, void* user_data);
    static void onAfterRtcEngineInitialize(const char* room_uuid, void* user_data);
    static void onBeforeRtcEngineRelease(const char* room_uuid, void* user_data);

    static void onMeetingStatusChanged(int32_t status, void* user_data);
    static void onMemberJoined(const NETenderMemberInfo* member, void* user_data);
    static void onMemberLeft(const NETenderMemberInfo* member, void* user_data);
    static void onHostChanged(const char* host_uuid, void* user_data);
    static void onMemberAudioMuteChanged(const char* user_uuid, int32_t muted, void* user_data);
    static void onMemberVideoMuteChanged(const char* user_uuid, int32_t muted, void* user_data);
    static void onRoomEnded(const char* reason, const char* extra, void* user_data);
    static void onMemberAudioConnectStateChanged(const NETenderMemberInfo* member, int32_t is_audio_connected, void* user_data);
    static void onMemberJoinChatroom(const NETenderMemberInfo* members, uint32_t count, void* user_data);
    static void onMemberLeaveChatroom(const NETenderMemberInfo* members, uint32_t count, void* user_data);
    static void onMemberScreenShareStateChanged(
        const NETenderMemberInfo* member, int32_t is_sharing, const NETenderMemberInfo* operate_by, void* user_data);
    static void onReceiveChatroomMessages(const NETenderRoomChatMessage* messages, uint32_t count, void* user_data);
    static void onMemberChatBanStateChanged(
        const NETenderMemberInfo* member, int32_t banned, int32_t duration, const char* notify_ext, const NETenderMemberInfo* operate_by, void* user_data);

    static void onCustomMessageReceived(const NETenderCustomMessage* message, void* user_data);
    static void onSessionMessageReceived(const NETenderCustomSessionMessage* data, void* user_data);
    static void onSessionMessageRecentChanged(const NETenderRecentSession* sessions, uint32_t count, void* user_data);
    static void onSessionMessageDeleted(const NETenderCustomSessionMessage* data, void* user_data);
    static void onDeleteAllSessionMessage(const char* session_id, int32_t session_type, void* user_data);

    static void onWaitingRoomMemberJoin(const NETenderWaitingRoomMember* member, int32_t reason, void* user_data);
    static void onWaitingRoomMemberLeave(const char* user_uuid, int32_t reason, void* user_data);
    static void onWaitingRoomMemberAdmitted(const char* user_uuid, void* user_data);
    static void onWaitingRoomMyStatusChanged(int32_t status, int32_t reason, void* user_data);
    static void onWaitingRoomMemberNameChanged(const char* user_uuid, const char* name, const NETenderMemberInfo* operate_by, void* user_data);
    static void onWaitingRoomInfoUpdated(const NETenderWaitingRoomInfo* info, void* user_data);
    static void onWaitingRoomAllMembersKicked(void* user_data);
    static void onWaitingRoomManagersUpdated(const NETenderMemberInfo* managers, uint32_t count, void* user_data);

    // 回调数据结构
    struct CallbackData {
        MainWindow* window;
        QString operation;
        QObject* target;
        const char* slot;
    };
};

#endif  // MAINWINDOW_H
