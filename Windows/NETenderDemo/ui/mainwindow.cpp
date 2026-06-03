#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "neutil.h"

struct CallbackData {
    MainWindow* window;
    QString operation;
    QObject* target;
    const char* slot;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sdkInitialized(false)
    , sdkHandle(nullptr)
    , meetingManager(nullptr)
    , accountManager(nullptr)
    , audioManager(nullptr)
    , videoManager(nullptr)
    , memberManager(nullptr)
    , screenShareManager(nullptr)
    , chatroomManager(nullptr)
    , inviteManager(nullptr)
    , preMeetingManager(nullptr)
    , securityManager(nullptr)
    , waitingRoomManager(nullptr)
    , speakerCombo(nullptr)
    , microphoneCombo(nullptr)
    , cameraCombo(nullptr)
    , screenSourceList(nullptr)
    , memberTable(nullptr)
    , chatHistory(nullptr)
    , messageEdit(nullptr)
    , usernameEdit(nullptr)
    , passwordEdit(nullptr)
    , accountIdEdit(nullptr)
    , tokenEdit(nullptr)
    , meetingSubjectEdit(nullptr)
    , waitingRoomCheck(nullptr)
    , recordingCheck(nullptr)
    , meetingNumEdit(nullptr)
    , joinNicknameEdit(nullptr)
    , joinUserUuidEdit(nullptr)
    , speakerSlider(nullptr)
    , micSlider(nullptr)
    , videoPreviewLabel(nullptr)
    , systemAudioCheck(nullptr)
    , searchPhoneEdit(nullptr)
    , userUuidsEdit(nullptr)
    , searchResultsList(nullptr)
    , scheduleSubjectEdit(nullptr)
    , scheduleStartTimeEdit(nullptr)
    , scheduleEndTimeEdit(nullptr)
    , scheduleWaitingRoomCheck(nullptr)
    , scheduleRecordingCheck(nullptr)
    , editMeetingIdEdit(nullptr)
    , cancelMeetingIdEdit(nullptr)
    , getMeetingNumEdit(nullptr)
    , meetingListTable(nullptr)
    , chatroomPermCombo(nullptr)
    , waitingRoomChatCheck(nullptr)
    , securityInfoEdit(nullptr)
    , waitingRoomUserUuidEdit(nullptr)
    , waitingRoomMemberTable(nullptr)
    , waitingRoomInfoEdit(nullptr)
    , localVideoWidget(nullptr)
    , remoteVideoWidget(nullptr)
    , subStreamVideoWidget(nullptr)
    , previewVideoWidget(nullptr)
    , autoScrollEnabled(true)
    , videoResolutionCombo(nullptr)
    , frameRateSpinBox(nullptr)
    , subscribeUserIdEdit(nullptr)
    , unsubscribeUserIdEdit(nullptr)
    , subscribeSubStreamUserIdEdit(nullptr)
    , unsubscribeSubStreamUserIdEdit(nullptr)
    , streamTypeCombo(nullptr)
    , enableAIDenoiseCheck(nullptr)
    , globalEventListenerRegistered(false)
    , meetingListenerRegistered(false)
    , messageChannelListenerRegistered(false)
    , waitingRoomListenerRegistered(false) {
    setWindowTitle("NETender Example");
    resize(1200, 800);

#if !defined(_WIN32) && !defined(__linux__)

    if (!canRecordScreen()) {
        showScreenRecordingPrompt();
    }

    authorizeCamera();
    authorizeMicrophone();
#endif

    setupUI();

    logMessage("NETender SDK 测试工具已启动");
    logMessage("请先初始化SDK，然后进行其他操作");
}

MainWindow::~MainWindow() {
    if (sdkInitialized) {
        uninitializeSDK();
    }
}

void MainWindow::setupUI() {
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    mainTabWidget = new QTabWidget(this);

    // 添加各个功能选项卡
    mainTabWidget->addTab(setupInitTab(), "SDK初始化");
    mainTabWidget->addTab(setupAccountTab(), "账户管理");
    mainTabWidget->addTab(setupMeetingTab(), "会议管理");
    mainTabWidget->addTab(setupAudioTab(), "音频管理");
    mainTabWidget->addTab(setupVideoTab(), "视频管理");
    mainTabWidget->addTab(setupScreenShareTab(), "屏幕共享");
    mainTabWidget->addTab(setupMemberTab(), "成员管理");
    mainTabWidget->addTab(setupChatTab(), "聊天室");
    mainTabWidget->addTab(setupInviteTab(), "邀请管理");
    mainTabWidget->addTab(setupPreMeetingTab(), "预会议管理");
    mainTabWidget->addTab(setupSecurityTab(), "安全管理");
    mainTabWidget->addTab(setupWaitingRoomTab(), "等候室管理");

    // 右侧：日志区域
    QWidget* logWidget = new QWidget(this);
    QVBoxLayout* logLayout = new QVBoxLayout(logWidget);
    logLayout->setContentsMargins(5, 5, 5, 5);

    // 日志标题和按钮区域
    QHBoxLayout* logHeaderLayout = new QHBoxLayout();
    QLabel* logTitle = new QLabel("操作日志", this);
    logTitle->setStyleSheet("font-weight: bold; font-size: 14px;");

    clearLogButton = new QPushButton("清空", this);
    saveLogButton = new QPushButton("保存", this);
    autoScrollButton = new QPushButton("自动滚动: 开", this);
    autoScrollButton->setCheckable(true);
    autoScrollButton->setChecked(true);

    logHeaderLayout->addWidget(logTitle);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(clearLogButton);
    logHeaderLayout->addWidget(saveLogButton);
    logHeaderLayout->addWidget(autoScrollButton);

    // 日志文本区域
    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setFont(QFont("Consolas", 9));
    logTextEdit->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #555; }");

    // 搜索区域（可选功能）
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLineEdit* searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("搜索日志...");
    QPushButton* searchButton = new QPushButton("搜索", this);

    searchLayout->addWidget(new QLabel("搜索:"));
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    // 组装日志区域
    logLayout->addLayout(logHeaderLayout);
    logLayout->addLayout(searchLayout);
    logLayout->addWidget(logTextEdit);

    // 设置分割条
    mainSplitter->addWidget(mainTabWidget);
    mainSplitter->addWidget(logWidget);

    // 设置分割比例：左侧70%，右侧30%
    mainSplitter->setStretchFactor(0, 7);
    mainSplitter->setStretchFactor(1, 3);

    // 设置最小宽度，防止被拖拽得太小
    mainTabWidget->setMinimumWidth(600);
    logWidget->setMinimumWidth(300);

    // 连接信号槽
    connect(clearLogButton, &QPushButton::clicked, this, &MainWindow::clearLog);
    connect(saveLogButton, &QPushButton::clicked, this, &MainWindow::saveLog);
    connect(autoScrollButton, &QPushButton::toggled, this, &MainWindow::toggleAutoScroll);
    connect(searchButton, &QPushButton::clicked, [this, searchEdit]() {
        this->searchLog(searchEdit->text());
    });
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp).arg(message);

    logTextEdit->append(formattedMessage);

    if (autoScrollEnabled) {
        QTextCursor cursor = logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        logTextEdit->setTextCursor(cursor);
    }
}

void MainWindow::clearLog() {
    logTextEdit->clear();
    logMessage("日志已清空");
}

void MainWindow::saveLog() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存日志文件",
        QString("netender_log_%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")), "文本文件 (*.log);;所有文件 (*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << logTextEdit->toPlainText();
            file.close();
            logMessage("日志已保存到: " + fileName);
        } else {
            QMessageBox::warning(this, "保存失败", "无法保存日志文件");
        }
    }
}

void MainWindow::toggleAutoScroll(bool enabled) {
    autoScrollEnabled = enabled;
    autoScrollButton->setText(enabled ? "自动滚动: 开" : "自动滚动: 关");
}

void MainWindow::searchLog(const QString& searchText) {
    if (searchText.isEmpty()) {
        return;
    }

    QTextDocument* document = logTextEdit->document();
    QTextCursor cursor(document);

    // 清除之前的高亮
    QTextCharFormat normalFormat;
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(normalFormat);
    cursor.clearSelection();

    // 高亮搜索文本
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::yellow);
    highlightFormat.setForeground(Qt::black);

    bool found = false;
    cursor = document->find(searchText, 0);

    while (!cursor.isNull()) {
        found = true;
        cursor.mergeCharFormat(highlightFormat);
        cursor = document->find(searchText, cursor);
    }

    if (found) {
        // 滚动到第一个匹配项
        cursor = document->find(searchText, 0);
        if (!cursor.isNull()) {
            logTextEdit->setTextCursor(cursor);
        }
    } else {
        logMessage(QString("未找到: %1").arg(searchText));
    }
}

void MainWindow::setupSDKCallbacks() {
    if (!sdkInitialized) {
        return;
    }

    if (!globalEventListenerRegistered) {
        globalEventListener = NETenderGlobalEventListener{};
        globalEventListener.before_rtc_engine_initialize = &MainWindow::onBeforeRtcEngineInitialize;
        globalEventListener.after_rtc_engine_initialize = &MainWindow::onAfterRtcEngineInitialize;
        globalEventListener.before_rtc_engine_release = &MainWindow::onBeforeRtcEngineRelease;
        globalEventListener.user_data = this;

        int32_t result = NETender_AddGlobalEventListener(&globalEventListener);
        if (result == 0) {
            globalEventListenerRegistered = true;
            logMessage("已注册全局事件监听回调示例");
        } else {
            logMessage(QString("注册全局事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (meetingManager && !meetingListenerRegistered) {
        meetingListener = NETenderMeetingListener{};
        meetingListener.on_meeting_status_changed = &MainWindow::onMeetingStatusChanged;
        meetingListener.on_member_joined = &MainWindow::onMemberJoined;
        meetingListener.on_member_left = &MainWindow::onMemberLeft;
        meetingListener.on_host_changed = &MainWindow::onHostChanged;
        meetingListener.on_member_audio_mute_changed = &MainWindow::onMemberAudioMuteChanged;
        meetingListener.on_member_video_mute_changed = &MainWindow::onMemberVideoMuteChanged;
        meetingListener.on_room_ended = &MainWindow::onRoomEnded;
        meetingListener.on_member_audio_connect_state_changed = &MainWindow::onMemberAudioConnectStateChanged;
        meetingListener.on_member_join_chatroom = &MainWindow::onMemberJoinChatroom;
        meetingListener.on_member_leave_chatroom = &MainWindow::onMemberLeaveChatroom;
        meetingListener.on_member_screen_share_state_changed = &MainWindow::onMemberScreenShareStateChanged;
        meetingListener.on_receive_chatroom_messages = &MainWindow::onReceiveChatroomMessages;
        meetingListener.on_member_chat_ban_state_changed = &MainWindow::onMemberChatBanStateChanged;
        meetingListener.user_data = this;

        int32_t result = NETender_Meeting_AddMeetingListener(meetingManager, &meetingListener);
        if (result == 0) {
            meetingListenerRegistered = true;
            logMessage("已注册会议事件监听回调示例");
        } else {
            logMessage(QString("注册会议事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (meetingManager && !messageChannelListenerRegistered) {
        messageChannelListener = NETenderMessageChannelListener{};
        messageChannelListener.on_custom_message_received = &MainWindow::onCustomMessageReceived;
        messageChannelListener.on_session_message_received = &MainWindow::onSessionMessageReceived;
        messageChannelListener.on_session_message_recent_changed = &MainWindow::onSessionMessageRecentChanged;
        messageChannelListener.on_session_message_deleted = &MainWindow::onSessionMessageDeleted;
        messageChannelListener.on_delete_all_session_message = &MainWindow::onDeleteAllSessionMessage;
        messageChannelListener.user_data = this;

        int32_t result = NETender_Meeting_AddMessageChannelListener(meetingManager, &messageChannelListener);
        if (result == 0) {
            messageChannelListenerRegistered = true;
            logMessage("已注册消息通道事件监听回调示例");
        } else {
            logMessage(QString("注册消息通道事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (waitingRoomManager && !waitingRoomListenerRegistered) {
        waitingRoomListener = NETenderWaitingRoomListener{};
        waitingRoomListener.on_member_join = &MainWindow::onWaitingRoomMemberJoin;
        waitingRoomListener.on_member_leave = &MainWindow::onWaitingRoomMemberLeave;
        waitingRoomListener.on_member_admitted = &MainWindow::onWaitingRoomMemberAdmitted;
        waitingRoomListener.on_my_status_changed = &MainWindow::onWaitingRoomMyStatusChanged;
        waitingRoomListener.on_member_name_changed = &MainWindow::onWaitingRoomMemberNameChanged;
        waitingRoomListener.on_info_updated = &MainWindow::onWaitingRoomInfoUpdated;
        waitingRoomListener.on_all_members_kicked = &MainWindow::onWaitingRoomAllMembersKicked;
        waitingRoomListener.on_managers_updated = &MainWindow::onWaitingRoomManagersUpdated;
        waitingRoomListener.user_data = this;

        int32_t result = NETender_WaitingRoom_AddListener(waitingRoomManager, &waitingRoomListener);
        if (result == 0) {
            waitingRoomListenerRegistered = true;
            logMessage("已注册等候室事件监听回调示例");
        } else {
            logMessage(QString("注册等候室事件监听失败，错误代码: %1").arg(result));
        }
    }

    logMessage("非高频回调注册流程已完成；RTC 统计等高频回调示例未在 UI 中持续打印");
}

void MainWindow::unregisterSDKCallbacks() {
    if (waitingRoomListenerRegistered && waitingRoomManager) {
        int32_t result = NETender_WaitingRoom_RemoveListener(waitingRoomManager, &waitingRoomListener);
        if (result == 0) {
            waitingRoomListenerRegistered = false;
        } else {
            logMessage(QString("移除等候室事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (messageChannelListenerRegistered && meetingManager) {
        int32_t result = NETender_Meeting_RemoveMessageChannelListener(meetingManager, &messageChannelListener);
        if (result == 0) {
            messageChannelListenerRegistered = false;
        } else {
            logMessage(QString("移除消息通道事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (meetingListenerRegistered && meetingManager) {
        int32_t result = NETender_Meeting_RemoveMeetingListener(meetingManager, &meetingListener);
        if (result == 0) {
            meetingListenerRegistered = false;
        } else {
            logMessage(QString("移除会议事件监听失败，错误代码: %1").arg(result));
        }
    }

    if (globalEventListenerRegistered) {
        int32_t result = NETender_RemoveGlobalEventListener(&globalEventListener);
        if (result == 0) {
            globalEventListenerRegistered = false;
        } else {
            logMessage(QString("移除全局事件监听失败，错误代码: %1").arg(result));
        }
    }
}

void MainWindow::postListenerLog(void* user_data, const QString& message) {
    MainWindow* window = static_cast<MainWindow*>(user_data);
    if (!window) {
        return;
    }

    QMetaObject::invokeMethod(window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("[回调] %1").arg(message)));
}

QString MainWindow::safeText(const char* text, const QString& fallback) {
    return text && text[0] != '\0' ? QString::fromUtf8(text) : fallback;
}

QString MainWindow::boolStateText(int32_t enabled) {
    if (enabled == 1) {
        return "是";
    }
    if (enabled == 0) {
        return "否";
    }
    return QString("未知(%1)").arg(enabled);
}

QString MainWindow::memberSummary(const NETenderMemberInfo* member) {
    if (!member) {
        return "未知成员";
    }

    return QString("uuid=%1 name=%2").arg(safeText(member->uuid)).arg(safeText(member->name));
}

QString MainWindow::chatMessageSummary(const NETenderRoomChatMessage* message) {
    if (!message) {
        return "未知消息";
    }

    return QString("fromUuid=%1 fromName=%2 type=%3")
        .arg(safeText(message->from_user_uuid))
        .arg(safeText(message->from_nick))
        .arg(static_cast<int>(message->message_type));
}

void MainWindow::onBeforeRtcEngineInitialize(const char* room_uuid, void* user_data) {
    postListenerLog(user_data, QString("全局事件 beforeRtcEngineInitialize, roomUuid=%1").arg(safeText(room_uuid)));
}

void MainWindow::onAfterRtcEngineInitialize(const char* room_uuid, void* user_data) {
    postListenerLog(user_data, QString("全局事件 afterRtcEngineInitialize, roomUuid=%1").arg(safeText(room_uuid)));
}

void MainWindow::onBeforeRtcEngineRelease(const char* room_uuid, void* user_data) {
    postListenerLog(user_data, QString("全局事件 beforeRtcEngineRelease, roomUuid=%1").arg(safeText(room_uuid)));
}

void MainWindow::onMeetingStatusChanged(int32_t status, void* user_data) {
    postListenerLog(user_data, QString("会议状态回调 onMeetingStatusChanged, status=%1").arg(status));
}

void MainWindow::onMemberJoined(const NETenderMemberInfo* member, void* user_data) {
    postListenerLog(user_data, QString("会议事件 onMemberJoined, member=%1").arg(memberSummary(member)));
}

void MainWindow::onMemberLeft(const NETenderMemberInfo* member, void* user_data) {
    postListenerLog(user_data, QString("会议事件 onMemberLeft, member=%1").arg(memberSummary(member)));
}

void MainWindow::onHostChanged(const char* host_uuid, void* user_data) {
    postListenerLog(user_data, QString("会议事件 onHostChanged, hostUuid=%1").arg(safeText(host_uuid)));
}

void MainWindow::onMemberAudioMuteChanged(const char* user_uuid, int32_t muted, void* user_data) {
    postListenerLog(user_data, QString("会议事件 onMemberAudioMuteChanged, userUuid=%1, muted=%2").arg(safeText(user_uuid)).arg(boolStateText(muted)));
}

void MainWindow::onMemberVideoMuteChanged(const char* user_uuid, int32_t muted, void* user_data) {
    postListenerLog(user_data, QString("会议事件 onMemberVideoMuteChanged, userUuid=%1, muted=%2").arg(safeText(user_uuid)).arg(boolStateText(muted)));
}

void MainWindow::onRoomEnded(const char* reason, const char* extra, void* user_data) {
    Q_UNUSED(extra);
    postListenerLog(user_data, QString("会议事件 onRoomEnded, reason=%1").arg(safeText(reason)));
}

void MainWindow::onMemberAudioConnectStateChanged(const NETenderMemberInfo* member, int32_t is_audio_connected, void* user_data) {
    postListenerLog(user_data,
        QString("会议事件 onMemberAudioConnectStateChanged, member=%1, connected=%2").arg(memberSummary(member)).arg(boolStateText(is_audio_connected)));
}

void MainWindow::onMemberJoinChatroom(const NETenderMemberInfo* members, uint32_t count, void* user_data) {
    QString firstMember = count > 0 ? memberSummary(members) : "无";
    postListenerLog(user_data, QString("会议事件 onMemberJoinChatroom, count=%1, first=%2").arg(count).arg(firstMember));
}

void MainWindow::onMemberLeaveChatroom(const NETenderMemberInfo* members, uint32_t count, void* user_data) {
    QString firstMember = count > 0 ? memberSummary(members) : "无";
    postListenerLog(user_data, QString("会议事件 onMemberLeaveChatroom, count=%1, first=%2").arg(count).arg(firstMember));
}

void MainWindow::onMemberScreenShareStateChanged(
    const NETenderMemberInfo* member, int32_t is_sharing, const NETenderMemberInfo* operate_by, void* user_data) {
    postListenerLog(user_data,
        QString("会议事件 onMemberScreenShareStateChanged, member=%1, sharing=%2, op=%3")
            .arg(memberSummary(member))
            .arg(boolStateText(is_sharing))
            .arg(memberSummary(operate_by)));
}

void MainWindow::onReceiveChatroomMessages(const NETenderRoomChatMessage* messages, uint32_t count, void* user_data) {
    QString firstMessage = count > 0 ? chatMessageSummary(messages) : "无";
    postListenerLog(user_data, QString("会议事件 onReceiveChatroomMessages, count=%1, first=%2").arg(count).arg(firstMessage));
}

void MainWindow::onMemberChatBanStateChanged(
    const NETenderMemberInfo* member, int32_t banned, int32_t duration, const char* notify_ext, const NETenderMemberInfo* operate_by, void* user_data) {
    Q_UNUSED(notify_ext);
    postListenerLog(user_data,
        QString("会议事件 onMemberChatBanStateChanged, member=%1, banned=%2, duration=%3, op=%4")
            .arg(memberSummary(member))
            .arg(boolStateText(banned))
            .arg(duration)
            .arg(memberSummary(operate_by)));
}

void MainWindow::onCustomMessageReceived(const NETenderCustomMessage* message, void* user_data) {
    Q_UNUSED(message);
    Q_UNUSED(user_data);
    // C API currently does not expose commandId/type/source. RoomKit internal
    // commands can arrive here, so the demo avoids UI logging for this callback.
}

void MainWindow::onSessionMessageReceived(const NETenderCustomSessionMessage* data, void* user_data) {
    if (!data) {
        postListenerLog(user_data, "消息通道 onSessionMessageReceived, data=空");
        return;
    }

    QString log = QString("消息通道 onSessionMessageReceived, sessionId=%1, sessionType=%2, messageId=%3")
                      .arg(safeText(data->sessionId))
                      .arg(data->sessionType)
                      .arg(safeText(data->messageId, "空"));
    if (data->timestamp > 0) {
        log += QString(", timestamp=%1").arg(data->timestamp);
    }
    postListenerLog(user_data, log);
}

void MainWindow::onSessionMessageRecentChanged(const NETenderRecentSession* sessions, uint32_t count, void* user_data) {
    QString firstSession = "无";
    if (sessions && count > 0) {
        firstSession = QString("sessionId=%1, sessionType=%2, unread=%3")
                           .arg(safeText(sessions[0].sessionId))
                           .arg(sessions[0].sessionType)
                           .arg(sessions[0].unreadCount);
    }
    postListenerLog(user_data, QString("消息通道 onSessionMessageRecentChanged, count=%1, first=%2").arg(count).arg(firstSession));
}

void MainWindow::onSessionMessageDeleted(const NETenderCustomSessionMessage* data, void* user_data) {
    if (!data) {
        postListenerLog(user_data, "消息通道 onSessionMessageDeleted, data=空");
        return;
    }

    postListenerLog(user_data,
        QString("消息通道 onSessionMessageDeleted, sessionId=%1, sessionType=%2, messageId=%3")
            .arg(safeText(data->sessionId))
            .arg(data->sessionType)
            .arg(safeText(data->messageId)));
}

void MainWindow::onDeleteAllSessionMessage(const char* session_id, int32_t session_type, void* user_data) {
    postListenerLog(user_data,
        QString("消息通道 onDeleteAllSessionMessage, sessionId=%1, sessionType=%2").arg(safeText(session_id)).arg(session_type));
}

void MainWindow::onWaitingRoomMemberJoin(const NETenderWaitingRoomMember* member, int32_t reason, void* user_data) {
    if (!member) {
        postListenerLog(user_data, QString("等候室事件 onMemberJoin, member=空, reason=%1").arg(reason));
        return;
    }

    postListenerLog(user_data,
        QString("等候室事件 onMemberJoin, userUuid=%1, name=%2, status=%3, reason=%4")
            .arg(safeText(member->user_uuid))
            .arg(safeText(member->user_name))
            .arg(member->status)
            .arg(reason));
}

void MainWindow::onWaitingRoomMemberLeave(const char* user_uuid, int32_t reason, void* user_data) {
    postListenerLog(user_data, QString("等候室事件 onMemberLeave, userUuid=%1, reason=%2").arg(safeText(user_uuid)).arg(reason));
}

void MainWindow::onWaitingRoomMemberAdmitted(const char* user_uuid, void* user_data) {
    postListenerLog(user_data, QString("等候室事件 onMemberAdmitted, userUuid=%1").arg(safeText(user_uuid)));
}

void MainWindow::onWaitingRoomMyStatusChanged(int32_t status, int32_t reason, void* user_data) {
    postListenerLog(user_data, QString("等候室状态回调 onMyStatusChanged, status=%1, reason=%2").arg(status).arg(reason));
}

void MainWindow::onWaitingRoomMemberNameChanged(const char* user_uuid, const char* name, const NETenderMemberInfo* operate_by, void* user_data) {
    postListenerLog(user_data,
        QString("等候室事件 onMemberNameChanged, userUuid=%1, name=%2, op=%3")
            .arg(safeText(user_uuid))
            .arg(safeText(name))
            .arg(memberSummary(operate_by)));
}

void MainWindow::onWaitingRoomInfoUpdated(const NETenderWaitingRoomInfo* info, void* user_data) {
    if (!info) {
        postListenerLog(user_data, "等候室事件 onInfoUpdated, info=空");
        return;
    }

    postListenerLog(user_data,
        QString("等候室事件 onInfoUpdated, memberCount=%1, enabled=%2")
            .arg(info->member_count)
            .arg(boolStateText(info->is_enabled)));
}

void MainWindow::onWaitingRoomAllMembersKicked(void* user_data) {
    postListenerLog(user_data, "等候室事件 onAllMembersKicked");
}

void MainWindow::onWaitingRoomManagersUpdated(const NETenderMemberInfo* managers, uint32_t count, void* user_data) {
    QString firstManager = count > 0 ? memberSummary(managers) : "无";
    postListenerLog(user_data, QString("等候室事件 onManagersUpdated, count=%1, first=%2").arg(count).arg(firstManager));
}

void MainWindow::asyncCallback(int32_t code, const char* message, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        QString result = QString("操作[%1]: %2 (代码: %3)").arg(data->operation).arg(message ? message : "成功").arg(code);
        QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));
    }

    if (data) {
        delete data;
        data = nullptr;
    }
}

void MainWindow::accountInfoCallback(int32_t code, const char* message, NETenderAccountInfo* account_info, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        QString result;

        if (code == 0 && account_info) {
            result = QString("操作[%1]: 成功 - 用户: %2, UUID: %3")
                         .arg(data->operation)
                         .arg(account_info->nickname ? account_info->nickname : "未知")
                         .arg(account_info->userUuid ? account_info->userUuid : "未知");
            NETender_FreeAccountInfo(account_info);
        } else {
            result = QString("操作[%1]: %2 (代码: %3)").arg(data->operation).arg(message ? message : "失败").arg(code);
        }
        QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));
    }

    //    if (data) {
    //        delete data;
    //        data = nullptr;
    //    }
}

void MainWindow::deviceListCallback(int32_t code, const char* message, NETenderDeviceInfo* devices, uint32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);
    if (data && data->window) {
        if (code == 0) {
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection,
                Q_ARG(QString, QString("操作[%1]: 获取到 %2 个设备").arg(data->operation).arg(count)));

            // 将更新UI和释放设备列表的操作都放在主线程中执行
            QMetaObject::invokeMethod(
                data->window,
                [data, devices, count]() {
                    // 根据操作类型判断更新哪个设备列表
                    if (data->operation.contains("扬声器")) {
                        // 更新扬声器设备列表
                        if (data->window->speakerCombo) {
                            data->window->speakerCombo->clear();

                            for (uint32_t i = 0; i < count; ++i) {
                                QString deviceName = devices[i].device_name ? devices[i].device_name : "未知设备";
                                QString deviceId = devices[i].device_id ? devices[i].device_id : "";

                                if (!deviceId.isEmpty()) {
                                    data->window->speakerCombo->addItem(deviceName, deviceId);
                                }
                            }

                            if (count > 0) {
                                data->window->speakerCombo->setCurrentIndex(0);
                            }
                        }
                    } else if (data->operation.contains("麦克风")) {
                        // 更新麦克风设备列表
                        if (data->window->microphoneCombo) {
                            data->window->microphoneCombo->clear();

                            for (uint32_t i = 0; i < count; ++i) {
                                QString deviceName = devices[i].device_name ? devices[i].device_name : "未知设备";
                                QString deviceId = devices[i].device_id ? devices[i].device_id : "";

                                if (!deviceId.isEmpty()) {
                                    data->window->microphoneCombo->addItem(deviceName, deviceId);
                                }
                            }

                            if (count > 0) {
                                data->window->microphoneCombo->setCurrentIndex(0);
                            }
                        }
                    } else if (data->operation.contains("视频") || data->operation.contains("摄像头")) {
                        // 更新摄像头设备列表
                        if (data->window->cameraCombo) {
                            data->window->cameraCombo->clear();

                            for (uint32_t i = 0; i < count; ++i) {
                                QString deviceName = devices[i].device_name ? devices[i].device_name : "未知设备";
                                QString deviceId = devices[i].device_id ? devices[i].device_id : "";

                                if (!deviceId.isEmpty()) {
                                    data->window->cameraCombo->addItem(deviceName, deviceId);
                                }
                            }

                            if (count > 0) {
                                data->window->cameraCombo->setCurrentIndex(0);
                            }
                        }
                    }

                    // 在更新完UI后释放设备列表
                    NETender_FreeDeviceList(devices, count);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }
    }
}

void MainWindow::screenSourceCallback(int32_t code, const char* message, NETenderScreenCaptureSource* sources, uint32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0) {
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection,
                Q_ARG(QString, QString("操作[%1]: 获取到 %2 个共享源").arg(data->operation).arg(count)));

            // 更新屏幕共享源列表，并在完成后释放资源
            QMetaObject::invokeMethod(
                data->window,
                [data, sources, count]() {
                    if (data->window->screenSourceList) {
                        data->window->screenSourceList->clear();

                        for (uint32_t i = 0; i < count; ++i) {
                            QString sourceName = sources[i].source_name ? sources[i].source_name : "未知源";
                            sourceName = sourceName + QString(" (ID: %1)").arg(QString::number(sources[i].source_id));
                            sourceName = sourceName + QString(" (source_type: %1)").arg(QString::number(sources[i].source_type));

                            QString sourceId = QString::number(sources[i].source_id);
                            QListWidgetItem* item = new QListWidgetItem(sourceName);
                            item->setData(Qt::UserRole, sourceId);  // 存储source_id
                            data->window->screenSourceList->addItem(item);
                        }

                        if (count > 0) {
                            data->window->screenSourceList->setCurrentRow(0);
                        }
                    }

                    // 在UI更新完成后释放资源
                    NETender_FreeScreenCaptureSourceList(sources, count);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));

            // 失败情况下也要释放资源
            if (sources && count > 0) {
                NETender_FreeScreenCaptureSourceList(sources, count);
            }
        }
    }
}

void MainWindow::chatMessageCallback(int32_t code, const char* message, NETenderChatMessage* chat_message, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        QString result;

        if (code == 0 && chat_message) {
            result =
                QString("操作[%1]: 消息发送成功 - ID: %2").arg(data->operation).arg(chat_message->message_uuid ? chat_message->message_uuid : "未知");
            NETender_FreeChatMessage(chat_message);
        } else {
            result = QString("操作[%1]: %2 (代码: %3)").arg(data->operation).arg(message ? message : "发送失败").arg(code);
        }

        QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));
    }

    //    if (data) {
    //        delete data;
    //        data = nullptr;
    //    }
}

void MainWindow::accountListCallback(int32_t code, const char* message, NETenderSearchAccountInfo* accounts, int32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0) {
            QString result = QString("操作[%1]: 获取到 %2 个账户").arg(data->operation).arg(count);
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));

            // 更新搜索结果列表
            QMetaObject::invokeMethod(
                data->window,
                [data, accounts, count]() {
                    if (data->window->searchResultsList) {
                        data->window->searchResultsList->clear();

                        for (int32_t i = 0; i < count; ++i) {
                            QString itemText = QString("%1 (%2)")
                                                   .arg(accounts[i].name ? accounts[i].name : "未知")
                                                   .arg(accounts[i].user_uuid ? accounts[i].user_uuid : "未知");
                            data->window->searchResultsList->addItem(itemText);
                        }
                    }

                    NETender_FreeAccountList(accounts, count);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }
    }
}

void MainWindow::meetingListCallback(int32_t code, const char* message, NETenderMeetingInfo* meetings, int32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0) {
            QString result = QString("操作[%1]: 获取到 %2 个会议").arg(data->operation).arg(count);
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));

            // 更新会议列表表格
            QMetaObject::invokeMethod(
                data->window,
                [data, meetings, count]() {
                    if (data->window->meetingListTable) {
                        data->window->meetingListTable->setRowCount(count);

                        for (int32_t i = 0; i < count; ++i) {
                            data->window->meetingListTable->setItem(i, 0, new QTableWidgetItem(QString::number(meetings[i].meeting_id)));
                            data->window->meetingListTable->setItem(
                                i, 1, new QTableWidgetItem(meetings[i].meeting_num ? meetings[i].meeting_num : ""));
                            data->window->meetingListTable->setItem(i, 2, new QTableWidgetItem(meetings[i].subject ? meetings[i].subject : ""));
                            data->window->meetingListTable->setItem(
                                i, 3, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(meetings[i].start_time).toString("yyyy-MM-dd hh:mm")));
                            data->window->meetingListTable->setItem(i, 4, new QTableWidgetItem(QString::number(meetings[i].state)));
                        }
                    }

                    // NETender_FreeMeetingList(meetings, count);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }
    }

    //    if (data) {
    //        delete data;
    //        data = nullptr;
    //    }
}

void MainWindow::meetingInfoCallback(int32_t code, const char* message, NETenderMeetingInfo* meeting_info, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        QString result;

        if (code == 0 && meeting_info) {
            result = QString("操作[%1]: 成功 - 会议: %2, 会议号: %3")
                         .arg(data->operation)
                         .arg(meeting_info->subject ? meeting_info->subject : "未知")
                         .arg(meeting_info->meeting_num ? meeting_info->meeting_num : "未知");
            NETender_FreePreMeetingInfo(meeting_info);
        } else {
            result = QString("操作[%1]: %2 (代码: %3)").arg(data->operation).arg(message ? message : "失败").arg(code);
        }

        QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));
    }
}

void MainWindow::scheduledMemberCallback(int32_t code, const char* message, NETenderScheduledMember* members, int32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0) {
            QString result = QString("操作[%1]: 获取到 %2 个预定成员").arg(data->operation).arg(count);
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));

            NETender_FreeScheduledMembers(members, count);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }

        // delete data;
    }
}

void MainWindow::chatroomPermissionCallback(int32_t code, const char* message, NETenderChatroomPermission* permission, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0 && permission) {
            QString result = QString("操作[%1]: 成功 - 聊天室权限: %2, 等候室聊天权限: %3")
                                 .arg(data->operation)
                                 .arg(permission->cr_perm ? permission->cr_perm : "未知")
                                 .arg(permission->wt_pr_chat ? permission->wt_pr_chat : "未知");
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));

            // 更新UI显示
            QMetaObject::invokeMethod(
                data->window,
                [data, permission]() {
                    if (data->window->securityInfoEdit) {
                        QString info = QString("聊天室权限: %1\n等候室聊天权限: %2")
                                           .arg(permission->cr_perm ? permission->cr_perm : "未知")
                                           .arg(permission->wt_pr_chat ? permission->wt_pr_chat : "未知");
                        data->window->securityInfoEdit->setPlainText(info);
                    }

                    NETender_FreeChatroomPermission(permission);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }
    }
}

void MainWindow::waitingRoomMemberCallback(int32_t code, const char* message, NETenderWaitingRoomMember* members, int32_t count, void* user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);

    if (data && data->window) {
        if (code == 0) {
            QString result = QString("操作[%1]: 获取到 %2 个等候室成员").arg(data->operation).arg(count);
            QMetaObject::invokeMethod(data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, result));

            // 更新等候室成员表格
            QMetaObject::invokeMethod(
                data->window,
                [data, members, count]() {
                    if (data->window->waitingRoomMemberTable) {
                        data->window->waitingRoomMemberTable->setRowCount(count);

                        for (int32_t i = 0; i < count; ++i) {
                            data->window->waitingRoomMemberTable->setItem(
                                i, 0, new QTableWidgetItem(members[i].user_uuid ? members[i].user_uuid : ""));
                            data->window->waitingRoomMemberTable->setItem(
                                i, 1, new QTableWidgetItem(members[i].user_name ? members[i].user_name : ""));
                            data->window->waitingRoomMemberTable->setItem(
                                i, 2, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(members[i].join_time).toString("yyyy-MM-dd hh:mm:ss")));
                        }
                    }

                    NETender_FreeWaitingRoomMemberList(members, count);
                },
                Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(
                data->window, "logMessage", Qt::QueuedConnection, Q_ARG(QString, QString("操作[%1]: 失败 - %2").arg(data->operation).arg(message)));
        }
    }
}

// SDK初始化选项卡
QWidget* MainWindow::setupInitTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QGroupBox* initGroup = new QGroupBox("SDK初始化配置");
    QFormLayout* formLayout = new QFormLayout(initGroup);

    appKeyEdit = new QLineEdit("");
    serverUrlEdit = new QLineEdit("");

    serverUrlEdit->setMinimumWidth(300);
    appKeyEdit->setMinimumWidth(300);
    localeCombo = new QComboBox();
    localeCombo->addItems({"zh-CN", "en-US", "ja-JP"});

    formLayout->addRow("App Key:", appKeyEdit);
    formLayout->addRow("服务器地址:", serverUrlEdit);
    formLayout->addRow("语言:", localeCombo);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* initBtn = new QPushButton("初始化SDK");
    QPushButton* uninitBtn = new QPushButton("反初始化SDK");
    QPushButton* checkBtn = new QPushButton("检查状态");

    buttonLayout->addWidget(initBtn);
    buttonLayout->addWidget(uninitBtn);
    buttonLayout->addWidget(checkBtn);

    layout->addWidget(initGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    connect(initBtn, &QPushButton::clicked, this, &MainWindow::initializeSDK);
    connect(uninitBtn, &QPushButton::clicked, this, &MainWindow::uninitializeSDK);
    connect(checkBtn, &QPushButton::clicked, [this]() {
        int initialized = NETender_IsInitialized();
        logMessage(QString("SDK初始化状态: %1").arg(initialized ? "已初始化" : "未初始化"));
    });

    return tab;
}

// 账户管理选项卡
QWidget* MainWindow::setupAccountTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 密码登录
    QGroupBox* pwdLoginGroup = new QGroupBox("密码登录");
    QFormLayout* pwdLayout = new QFormLayout(pwdLoginGroup);

    usernameEdit = new QLineEdit();
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);

    pwdLayout->addRow("用户名:", usernameEdit);
    pwdLayout->addRow("密码:", passwordEdit);

    QPushButton* pwdLoginBtn = new QPushButton("登录");
    pwdLayout->addRow(pwdLoginBtn);

    // Token登录
    QGroupBox* tokenLoginGroup = new QGroupBox("Token登录");
    QFormLayout* tokenLayout = new QFormLayout(tokenLoginGroup);

    accountIdEdit = new QLineEdit("");
    tokenEdit = new QLineEdit("");

    tokenLayout->addRow("账户ID:", accountIdEdit);
    tokenLayout->addRow("Token:", tokenEdit);

    QPushButton* tokenLoginBtn = new QPushButton("登录");
    tokenLayout->addRow(tokenLoginBtn);

    // 账户操作
    QGroupBox* accountOpsGroup = new QGroupBox("账户操作");
    QHBoxLayout* opsLayout = new QHBoxLayout(accountOpsGroup);

    QPushButton* getInfoBtn = new QPushButton("获取账户信息");
    QPushButton* updateNickBtn = new QPushButton("更新昵称");
    QPushButton* logoutBtn = new QPushButton("退出登录");

    opsLayout->addWidget(getInfoBtn);
    opsLayout->addWidget(updateNickBtn);
    opsLayout->addWidget(logoutBtn);

    layout->addWidget(pwdLoginGroup);
    layout->addWidget(tokenLoginGroup);
    layout->addWidget(accountOpsGroup);
    layout->addStretch();

    connect(pwdLoginBtn, &QPushButton::clicked, this, &MainWindow::loginByPassword);
    connect(tokenLoginBtn, &QPushButton::clicked, this, &MainWindow::loginByToken);
    connect(getInfoBtn, &QPushButton::clicked, this, &MainWindow::getAccountInfo);
    connect(updateNickBtn, &QPushButton::clicked, this, &MainWindow::updateNickname);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::logout);

    return tab;
}

// 会议管理选项卡
QWidget* MainWindow::setupMeetingTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QGroupBox* createGroup = new QGroupBox("创建会议");
    QFormLayout* createLayout = new QFormLayout(createGroup);
    meetingSubjectEdit = new QLineEdit("测试会议");
    meetingPasswordEdit = new QLineEdit();
    meetingPasswordEdit->setPlaceholderText("可选密码");
    nickNameEdit = new QLineEdit();
    nickNameEdit->setPlaceholderText("昵称");
    noChatCheck = new QCheckBox("禁用聊天");
    noWhiteboardCheck = new QCheckBox("禁用白板");
    noCaptionsCheck = new QCheckBox("禁用字幕");
    orgNameEdit = new QLineEdit();
    orgNameEdit->setPlaceholderText("组织名称");
    avatarUrlEdit = new QLineEdit();
    avatarUrlEdit->setPlaceholderText("头像URL");
    joinTimeoutSpin = new QSpinBox();
    joinTimeoutSpin->setRange(-1, 3600000);
    joinTimeoutSpin->setValue(-1);
    joinTimeoutSpin->setSpecialValueText("不指定");
    sessionEdit = new QLineEdit();
    sessionEdit->setPlaceholderText("会话标识");
    meetingSubTypeCombo = new QComboBox();
    meetingSubTypeCombo->addItem("默认", 0);
    meetingSubTypeCombo->addItem("个人录制", 1);
    waitingRoomCheck = new QCheckBox("启用等候室");
    recordingCheck = new QCheckBox("启用云端录制");

    createLayout->addRow("会议主题:", meetingSubjectEdit);
    createLayout->addRow("会议密码:", meetingPasswordEdit);
    createLayout->addRow("昵称:", nickNameEdit);
    createLayout->addRow(noChatCheck);
    createLayout->addRow(noWhiteboardCheck);
    createLayout->addRow(noCaptionsCheck);
    createLayout->addRow("组织名称:", orgNameEdit);
    createLayout->addRow("头像URL:", avatarUrlEdit);
    createLayout->addRow("加入超时(ms):", joinTimeoutSpin);
    createLayout->addRow("会话标识:", sessionEdit);
    createLayout->addRow("会议子类型:", meetingSubTypeCombo);
    createLayout->addRow(waitingRoomCheck);
    createLayout->addRow(recordingCheck);

    QPushButton* createBtn = new QPushButton("创建会议");
    createLayout->addRow(createBtn);

    // 加入会议
    QGroupBox* joinGroup = new QGroupBox("加入会议");
    QFormLayout* joinLayout = new QFormLayout(joinGroup);

    meetingNumEdit = new QLineEdit();
    joinUserUuidEdit = new QLineEdit();
    joinNicknameEdit = new QLineEdit("测试用户");

    joinLayout->addRow("会议号:", meetingNumEdit);
    joinLayout->addRow("用户UUID:", joinUserUuidEdit);
    joinLayout->addRow("昵称:", joinNicknameEdit);

    QPushButton* joinBtn = new QPushButton("加入会议");
    joinLayout->addRow(joinBtn);

    // 会议操作
    QGroupBox* meetingOpsGroup = new QGroupBox("会议操作");
    QHBoxLayout* meetingOpsLayout = new QHBoxLayout(meetingOpsGroup);

    QPushButton* getMeetingInfoBtn = new QPushButton("获取会议信息");
    QPushButton* leaveBtn = new QPushButton("离开会议");
    QPushButton* endBtn = new QPushButton("结束会议");

    meetingOpsLayout->addWidget(getMeetingInfoBtn);
    meetingOpsLayout->addWidget(leaveBtn);
    meetingOpsLayout->addWidget(endBtn);

    layout->addWidget(createGroup);
    layout->addWidget(joinGroup);
    layout->addWidget(meetingOpsGroup);
    layout->addStretch();

    connect(createBtn, &QPushButton::clicked, this, &MainWindow::createMeeting);
    connect(joinBtn, &QPushButton::clicked, this, &MainWindow::joinMeeting);
    connect(getMeetingInfoBtn, &QPushButton::clicked, this, &MainWindow::getMeetingInfo);
    connect(leaveBtn, &QPushButton::clicked, this, &MainWindow::leaveMeeting);
    connect(endBtn, &QPushButton::clicked, this, &MainWindow::endMeeting);

    return tab;
}

// 音频管理选项卡
QWidget* MainWindow::setupAudioTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 设备列表
    QGroupBox* deviceGroup = new QGroupBox("音频设备");
    QVBoxLayout* deviceLayout = new QVBoxLayout(deviceGroup);

    QHBoxLayout* speakerLayout = new QHBoxLayout();

    speakerCombo = new QComboBox();
    QPushButton* refreshSpeakerBtn = new QPushButton("刷新扬声器");
    QPushButton* setSpeakerBtn = new QPushButton("设置扬声器");

    speakerLayout->addWidget(new QLabel("扬声器:"));
    speakerLayout->addWidget(speakerCombo, 1);
    speakerLayout->addWidget(refreshSpeakerBtn);
    speakerLayout->addWidget(setSpeakerBtn);

    QHBoxLayout* micLayout = new QHBoxLayout();
    microphoneCombo = new QComboBox();
    QPushButton* refreshMicBtn = new QPushButton("刷新麦克风");
    QPushButton* setMicBtn = new QPushButton("设置麦克风");

    micLayout->addWidget(new QLabel("麦克风:"));
    micLayout->addWidget(microphoneCombo, 1);
    micLayout->addWidget(refreshMicBtn);
    micLayout->addWidget(setMicBtn);

    deviceLayout->addLayout(speakerLayout);
    deviceLayout->addLayout(micLayout);

    // 音量控制
    QGroupBox* volumeGroup = new QGroupBox("音量控制");
    QFormLayout* volumeLayout = new QFormLayout(volumeGroup);

    speakerSlider = new QSlider(Qt::Horizontal);
    speakerSlider->setRange(0, 100);
    speakerSlider->setValue(50);
    QPushButton* setSpeakerVolumeBtn = new QPushButton("设置扬声器音量");

    micSlider = new QSlider(Qt::Horizontal);
    micSlider->setRange(0, 100);
    micSlider->setValue(50);
    QPushButton* setMicVolumeBtn = new QPushButton("设置麦克风音量");

    volumeLayout->addRow("扬声器音量:", speakerSlider);
    volumeLayout->addRow(setSpeakerVolumeBtn);
    volumeLayout->addRow("麦克风音量:", micSlider);
    volumeLayout->addRow(setMicVolumeBtn);

    // 音频控制
    QGroupBox* controlGroup = new QGroupBox("音频控制");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);

    // 基本音频控制
    QHBoxLayout* basicControlLayout = new QHBoxLayout();
    QPushButton* muteBtn = new QPushButton("静音");
    QPushButton* unmuteBtn = new QPushButton("取消静音");

    basicControlLayout->addWidget(muteBtn);
    basicControlLayout->addWidget(unmuteBtn);

    // 扬声器测试控制
    QHBoxLayout* speakerTestLayout = new QHBoxLayout();
    QPushButton* speakerTestBtn = new QPushButton("测试扬声器");
    QPushButton* stopSpeakerTestBtn = new QPushButton("停止扬声器测试");

    speakerTestLayout->addWidget(speakerTestBtn);
    speakerTestLayout->addWidget(stopSpeakerTestBtn);

    // 麦克风测试控制
    QHBoxLayout* micTestLayout = new QHBoxLayout();
    QPushButton* micTestBtn = new QPushButton("测试麦克风");
    QPushButton* stopMicTestBtn = new QPushButton("停止麦克风测试");

    micTestLayout->addWidget(micTestBtn);
    micTestLayout->addWidget(stopMicTestBtn);

    // 添加到主控制布局
    controlLayout->addLayout(basicControlLayout);
    controlLayout->addLayout(speakerTestLayout);
    controlLayout->addLayout(micTestLayout);

    // 音频效果控制
    QGroupBox* audioEffectGroup = new QGroupBox("音频效果");
    QFormLayout* audioEffectLayout = new QFormLayout(audioEffectGroup);

    audioEffectTypeCombo = new QComboBox();
    audioEffectTypeCombo->addItem("变声", 0);
    audioEffectTypeCombo->addItem("美声", 1);

    audioEffectValueSpin = new QSpinBox();
    audioEffectValueSpin->setRange(0, 12);
    audioEffectValueSpin->setValue(0);

    QPushButton* setAudioEffectBtn = new QPushButton("设置音频效果");

    audioEffectLayout->addRow("效果类型:", audioEffectTypeCombo);
    audioEffectLayout->addRow("效果值:", audioEffectValueSpin);
    audioEffectLayout->addRow(setAudioEffectBtn);

    // AI降噪控制
    QGroupBox* aiDenoiseGroup = new QGroupBox("AI音频增强");
    QFormLayout* aiDenoiseLayout = new QFormLayout(aiDenoiseGroup);

    enableAIDenoiseCheck = new QCheckBox("启用AI降噪");
    enableAIDenoiseCheck->setCheckable(true);

    aiDenoiseLayout->addRow(enableAIDenoiseCheck);

    layout->addWidget(deviceGroup);
    layout->addWidget(volumeGroup);
    layout->addWidget(controlGroup);
    layout->addWidget(audioEffectGroup);
    layout->addWidget(aiDenoiseGroup);
    layout->addStretch();

    connect(refreshSpeakerBtn, &QPushButton::clicked, this, &MainWindow::refreshSpeakerDevices);
    connect(refreshMicBtn, &QPushButton::clicked, this, &MainWindow::refreshMicrophoneDevices);
    connect(setSpeakerBtn, &QPushButton::clicked, this, &MainWindow::setSpeakerDevice);
    connect(setMicBtn, &QPushButton::clicked, this, &MainWindow::setMicrophoneDevice);
    connect(setSpeakerVolumeBtn, &QPushButton::clicked, this, &MainWindow::setSpeakerVolume);
    connect(setMicVolumeBtn, &QPushButton::clicked, this, &MainWindow::setMicrophoneVolume);
    connect(muteBtn, &QPushButton::clicked, this, &MainWindow::muteMyAudio);
    connect(unmuteBtn, &QPushButton::clicked, this, &MainWindow::unmuteMyAudio);
    connect(speakerTestBtn, &QPushButton::clicked, this, &MainWindow::startSpeakerTest);
    connect(stopSpeakerTestBtn, &QPushButton::clicked, this, &MainWindow::stopSpeakerTest);
    connect(micTestBtn, &QPushButton::clicked, this, &MainWindow::startMicrophoneTest);
    connect(stopMicTestBtn, &QPushButton::clicked, this, &MainWindow::stopMicrophoneTest);
    connect(setAudioEffectBtn, &QPushButton::clicked, this, &MainWindow::setAudioEffect);
    connect(enableAIDenoiseCheck, &QCheckBox::clicked, this, [this](bool checked) {
        if (checked) {
            NETender_Audio_EnableAIDenoise(audioManager, asyncCallback, new CallbackData{this, "启用AI降噪", nullptr, nullptr});
        } else {
            NETender_Audio_DisableAIDenoise(audioManager, asyncCallback, new CallbackData{this, "禁用AI降噪", nullptr, nullptr});
        }
    });

    return tab;
}

void MainWindow::setAudioEffect() {
    int type = audioEffectTypeCombo->currentData().toInt();
    int value = audioEffectValueSpin->value();
    NETender_Audio_SetAudioEffect(audioManager, type, value, asyncCallback, new CallbackData{this, "设置音频效果", nullptr, nullptr});
}
// 视频管理选项卡
QWidget* MainWindow::setupVideoTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    // 摄像头设备
    QGroupBox* cameraGroup = new QGroupBox("摄像头设备");
    QHBoxLayout* cameraLayout = new QHBoxLayout(cameraGroup);

    cameraCombo = new QComboBox();
    QPushButton* refreshCameraBtn = new QPushButton("刷新");
    QPushButton* setCameraBtn = new QPushButton("设置摄像头");

    cameraLayout->addWidget(new QLabel("摄像头:"));
    cameraLayout->addWidget(cameraCombo, 1);
    cameraLayout->addWidget(refreshCameraBtn);
    cameraLayout->addWidget(setCameraBtn);

    // 视频配置
    QGroupBox* videoConfigGroup = new QGroupBox("视频配置");
    QHBoxLayout* videoConfigLayout = new QHBoxLayout(videoConfigGroup);

    videoResolutionCombo = new QComboBox();
    videoResolutionCombo->addItem("160x120", 0);
    videoResolutionCombo->addItem("320x180", 1);
    videoResolutionCombo->addItem("320x240", 2);
    videoResolutionCombo->addItem("640x360", 3);
    videoResolutionCombo->addItem("640x480", 4);
    videoResolutionCombo->addItem("1280x720", 5);
    videoResolutionCombo->addItem("1920x1080", 6);
    videoResolutionCombo->setCurrentIndex(4);  // 默认640x480

    frameRateSpinBox = new QSpinBox();
    frameRateSpinBox->setRange(1, 60);
    frameRateSpinBox->setValue(15);  // 默认15fps

    QPushButton* setVideoProfileBtn = new QPushButton("设置视频配置");

    videoConfigLayout->addWidget(new QLabel("分辨率:"));
    videoConfigLayout->addWidget(videoResolutionCombo);
    videoConfigLayout->addWidget(new QLabel("帧率:"));
    videoConfigLayout->addWidget(frameRateSpinBox);
    videoConfigLayout->addWidget(setVideoProfileBtn);

    // 视频控制
    QGroupBox* videoControlGroup = new QGroupBox("视频控制");
    QHBoxLayout* videoControlLayout = new QHBoxLayout(videoControlGroup);

    QPushButton* startPreviewBtn = new QPushButton("开始预览");
    QPushButton* stopPreviewBtn = new QPushButton("停止预览");
    QPushButton* muteVideoBtn = new QPushButton("关闭视频");
    QPushButton* unmuteVideoBtn = new QPushButton("开启视频");

    videoControlLayout->addWidget(startPreviewBtn);
    videoControlLayout->addWidget(stopPreviewBtn);
    videoControlLayout->addWidget(muteVideoBtn);
    videoControlLayout->addWidget(unmuteVideoBtn);

    // 视频预览区域 - 使用网格布局，减少高度
    QGroupBox* previewGroup = new QGroupBox("视频窗口");
    QGridLayout* previewLayout = new QGridLayout(previewGroup);

    // 本地视频窗口 - 减小尺寸
    QGroupBox* localVideoGroup = new QGroupBox("本地视频");
    QVBoxLayout* localVideoLayout = new QVBoxLayout(localVideoGroup);
    localVideoWidget = new VideoRenderWidget();
    localVideoWidget->setMinimumSize(160, 120);  // 减小尺寸
    localVideoWidget->setMaximumSize(240, 180);
    localVideoWidget->setStyleSheet("border: 1px solid green; background-color: black;");
    QPushButton* setupLocalCanvasBtn = new QPushButton("设置本地画布");
    QPushButton* removeLocalCanvasBtn = new QPushButton("移除本地画布");
    localVideoLayout->addWidget(localVideoWidget);
    localVideoLayout->addWidget(setupLocalCanvasBtn);
    localVideoLayout->addWidget(removeLocalCanvasBtn);

    // 预览窗口 - 减小尺寸
    QGroupBox* previewVideoGroup = new QGroupBox("预览窗口");
    QVBoxLayout* previewVideoLayout = new QVBoxLayout(previewVideoGroup);
    previewVideoWidget = new VideoRenderWidget();
    previewVideoWidget->setMinimumSize(160, 120);  // 减小尺寸
    previewVideoWidget->setMaximumSize(240, 180);
    previewVideoWidget->setStyleSheet("border: 1px solid blue; background-color: black;");
    QPushButton* setupPreviewCanvasBtn = new QPushButton("设置预览画布");
    previewVideoLayout->addWidget(previewVideoWidget);
    previewVideoLayout->addWidget(setupPreviewCanvasBtn);

    // 远程视频窗口 - 减小尺寸
    QGroupBox* remoteVideoGroup = new QGroupBox("远程视频");
    QVBoxLayout* remoteVideoLayout = new QVBoxLayout(remoteVideoGroup);
    remoteVideoWidget = new VideoRenderWidget();
    remoteVideoWidget->setMinimumSize(160, 120);  // 减小尺寸
    remoteVideoWidget->setMaximumSize(240, 180);
    remoteVideoWidget->setStyleSheet("border: 1px solid red; background-color: black;");
    QLineEdit* remoteUserIdEdit = new QLineEdit();
    remoteUserIdEdit->setPlaceholderText("输入远程用户ID");
    QPushButton* setupRemoteCanvasBtn = new QPushButton("设置远程画布");
    QPushButton* removeRemoteCanvasBtn = new QPushButton("删除远程画布");  // 新增按钮
    remoteVideoLayout->addWidget(remoteVideoWidget);
    remoteVideoLayout->addWidget(remoteUserIdEdit);
    remoteVideoLayout->addWidget(setupRemoteCanvasBtn);
    remoteVideoLayout->addWidget(removeRemoteCanvasBtn);  // 添加到布局

    // 辅流视频窗口 - 减小尺寸
    QGroupBox* subStreamGroup = new QGroupBox("辅流视频");
    QVBoxLayout* subStreamLayout = new QVBoxLayout(subStreamGroup);
    subStreamVideoWidget = new VideoRenderWidget();
    subStreamVideoWidget->setMinimumSize(160, 120);  // 减小尺寸
    subStreamVideoWidget->setMaximumSize(240, 180);
    subStreamVideoWidget->setStyleSheet("border: 1px solid yellow; background-color: black;");
    QLineEdit* subStreamUserIdEdit = new QLineEdit();
    subStreamUserIdEdit->setPlaceholderText("输入用户ID");
    QPushButton* setupSubStreamCanvasBtn = new QPushButton("设置辅流画布");
    QPushButton* removeSubStreamCanvasBtn = new QPushButton("删除辅流画布");  // 新增按钮
    subStreamLayout->addWidget(subStreamVideoWidget);
    subStreamLayout->addWidget(subStreamUserIdEdit);
    subStreamLayout->addWidget(setupSubStreamCanvasBtn);
    subStreamLayout->addWidget(removeSubStreamCanvasBtn);  // 添加到布局

    // 将视频窗口排列在网格中 - 使用2x2布局
    previewLayout->addWidget(localVideoGroup, 0, 0);
    previewLayout->addWidget(previewVideoGroup, 0, 1);
    previewLayout->addWidget(remoteVideoGroup, 1, 0);
    previewLayout->addWidget(subStreamGroup, 1, 1);

    // 远程视频流订阅控制 - 使用紧凑布局
    QGroupBox* streamSubscriptionGroup = new QGroupBox("远程视频流订阅");
    QFormLayout* streamSubscriptionLayout = new QFormLayout(streamSubscriptionGroup);

    // 主流订阅
    subscribeUserIdEdit = new QLineEdit();
    subscribeUserIdEdit->setPlaceholderText("输入用户ID");
    QPushButton* subscribeBtn = new QPushButton("订阅主流");

    unsubscribeUserIdEdit = new QLineEdit();
    unsubscribeUserIdEdit->setPlaceholderText("输入用户ID");
    QPushButton* unsubscribeBtn = new QPushButton("取消订阅主流");

    // 辅流订阅
    subscribeSubStreamUserIdEdit = new QLineEdit();
    subscribeSubStreamUserIdEdit->setPlaceholderText("输入用户ID");
    QPushButton* subscribeSubBtn = new QPushButton("订阅辅流");

    unsubscribeSubStreamUserIdEdit = new QLineEdit();
    unsubscribeSubStreamUserIdEdit->setPlaceholderText("输入用户ID");
    QPushButton* unsubscribeSubBtn = new QPushButton("取消订阅辅流");

    // 流类型选择
    streamTypeCombo = new QComboBox();
    streamTypeCombo->addItem("HIGH", 0);
    streamTypeCombo->addItem("LOW", 1);
    streamTypeCombo->addItem("MEDIUM", 1);

    // 使用紧凑的表单布局
    streamSubscriptionLayout->addRow("订阅主流用户ID:", subscribeUserIdEdit);
    streamSubscriptionLayout->addRow(subscribeBtn);
    streamSubscriptionLayout->addRow("取消订阅主流用户ID:", unsubscribeUserIdEdit);
    streamSubscriptionLayout->addRow(unsubscribeBtn);
    streamSubscriptionLayout->addRow("订阅辅流用户ID:", subscribeSubStreamUserIdEdit);
    streamSubscriptionLayout->addRow(subscribeSubBtn);
    streamSubscriptionLayout->addRow("取消订阅辅流用户ID:", unsubscribeSubStreamUserIdEdit);
    streamSubscriptionLayout->addRow(unsubscribeSubBtn);
    streamSubscriptionLayout->addRow("流类型:", streamTypeCombo);

    // 将所有组件添加到滚动布局中
    scrollLayout->addWidget(cameraGroup);
    scrollLayout->addWidget(videoConfigGroup);
    scrollLayout->addWidget(videoControlGroup);
    scrollLayout->addWidget(previewGroup);
    scrollLayout->addWidget(streamSubscriptionGroup);
    scrollLayout->addStretch();

    // 设置滚动区域的内容
    scrollArea->setWidget(scrollWidget);

    // 将滚动区域添加到主布局
    layout->addWidget(scrollArea);

    // 连接信号槽
    connect(refreshCameraBtn, &QPushButton::clicked, this, &MainWindow::refreshVideoDevices);
    connect(setCameraBtn, &QPushButton::clicked, this, &MainWindow::setCameraDevice);
    connect(setVideoProfileBtn, &QPushButton::clicked, this, &MainWindow::setVideoProfile);
    connect(startPreviewBtn, &QPushButton::clicked, this, &MainWindow::startVideoPreview);
    connect(stopPreviewBtn, &QPushButton::clicked, this, &MainWindow::stopVideoPreview);
    connect(muteVideoBtn, &QPushButton::clicked, this, &MainWindow::muteMyVideoEx);
    connect(unmuteVideoBtn, &QPushButton::clicked, this, &MainWindow::unmuteMyVideoEx);

    // 连接画布设置按钮
    connect(setupLocalCanvasBtn, &QPushButton::clicked, this, &MainWindow::setupLocalVideoCanvas);
    connect(removeLocalCanvasBtn, &QPushButton::clicked, this, &MainWindow::removeLocalVideoCanvas);
    connect(setupPreviewCanvasBtn, &QPushButton::clicked, this, &MainWindow::setupVideoPreviewCanvas);

    // 使用lambda表达式处理带参数的远程画布设置
    connect(setupRemoteCanvasBtn, &QPushButton::clicked, [this, remoteUserIdEdit]() {
        QString userId = remoteUserIdEdit->text().trimmed();
        if (!userId.isEmpty()) {
            setupRemoteVideoCanvas(userId);
        } else {
            logMessage("请输入远程用户ID");
        }
    });

    // 新增：删除远程画布按钮连接
    connect(removeRemoteCanvasBtn, &QPushButton::clicked, [this, remoteUserIdEdit]() {
        QString userId = remoteUserIdEdit->text().trimmed();
        if (!userId.isEmpty()) {
            removeRemoteVideoCanvas(userId);
        } else {
            logMessage("请输入远程用户ID");
        }
    });

    connect(setupSubStreamCanvasBtn, &QPushButton::clicked, [this, subStreamUserIdEdit]() {
        QString userId = subStreamUserIdEdit->text().trimmed();
        if (!userId.isEmpty()) {
            setupRemoteVideoSubStreamCanvas(userId);
        } else {
            logMessage("请输入用户ID");
        }
    });

    // 新增：删除辅流画布按钮连接
    connect(removeSubStreamCanvasBtn, &QPushButton::clicked, [this, subStreamUserIdEdit]() {
        QString userId = subStreamUserIdEdit->text().trimmed();
        if (!userId.isEmpty()) {
            removeRemoteVideoSubStreamCanvas(userId);
        } else {
            logMessage("请输入用户ID");
        }
    });

    // 连接远程视频流订阅信号槽
    connect(subscribeBtn, &QPushButton::clicked, this, &MainWindow::subscribeRemoteVideoStream);
    connect(unsubscribeBtn, &QPushButton::clicked, this, &MainWindow::unsubscribeRemoteVideoStream);
    connect(subscribeSubBtn, &QPushButton::clicked, this, &MainWindow::subscribeRemoteVideoSubStream);
    connect(unsubscribeSubBtn, &QPushButton::clicked, this, &MainWindow::unsubscribeRemoteVideoSubStream);

    return tab;
}

// 屏幕共享选项卡
QWidget* MainWindow::setupScreenShareTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 共享源
    QGroupBox* sourceGroup = new QGroupBox("共享源");
    QVBoxLayout* sourceLayout = new QVBoxLayout(sourceGroup);

    screenSourceList = new QListWidget();
    QPushButton* refreshSourcesBtn = new QPushButton("刷新共享源");

    sourceLayout->addWidget(screenSourceList);
    sourceLayout->addWidget(refreshSourcesBtn);

    // 共享控制
    QGroupBox* shareControlGroup = new QGroupBox("共享控制");
    QHBoxLayout* shareControlLayout = new QHBoxLayout(shareControlGroup);

    QPushButton* startShareBtn = new QPushButton("开始共享");
    QPushButton* stopShareBtn = new QPushButton("停止共享");
    systemAudioCheck = new QCheckBox("共享系统音频");

    shareControlLayout->addWidget(startShareBtn);
    shareControlLayout->addWidget(stopShareBtn);
    shareControlLayout->addWidget(systemAudioCheck);

    layout->addWidget(sourceGroup);
    layout->addWidget(shareControlGroup);
    layout->addStretch();

    connect(refreshSourcesBtn, &QPushButton::clicked, this, &MainWindow::refreshScreenSources);
    connect(startShareBtn, &QPushButton::clicked, this, &MainWindow::startScreenShare);
    connect(stopShareBtn, &QPushButton::clicked, this, &MainWindow::stopScreenShare);

    return tab;
}

// 成员管理选项卡
QWidget* MainWindow::setupMemberTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 成员列表
    QGroupBox* memberGroup = new QGroupBox("会议成员");
    QVBoxLayout* memberLayout = new QVBoxLayout(memberGroup);

    memberTable = new QTableWidget(0, 5);
    memberTable->setHorizontalHeaderLabels({"用户UUID", "用户名", "角色", "音频状态", "视频状态"});
    memberTable->horizontalHeader()->setStretchLastSection(true);

    QPushButton* refreshMemberBtn = new QPushButton("刷新成员列表");

    memberLayout->addWidget(memberTable);
    memberLayout->addWidget(refreshMemberBtn);

    // 成员控制
    QGroupBox* memberControlGroup = new QGroupBox("成员控制");
    QHBoxLayout* memberControlLayout = new QHBoxLayout(memberControlGroup);

    QPushButton* muteMemberAudioBtn = new QPushButton("静音选中成员");
    QPushButton* unmuteMemberAudioBtn = new QPushButton("取消静音选中成员");
    QPushButton* muteAllAudioBtn = new QPushButton("静音所有成员");

    memberControlLayout->addWidget(muteMemberAudioBtn);
    memberControlLayout->addWidget(unmuteMemberAudioBtn);
    memberControlLayout->addWidget(muteAllAudioBtn);

    layout->addWidget(memberGroup);
    layout->addWidget(memberControlGroup);
    layout->addStretch();

    connect(refreshMemberBtn, &QPushButton::clicked, this, &MainWindow::refreshMemberList);
    connect(muteMemberAudioBtn, &QPushButton::clicked, this, &MainWindow::muteMemberAudio);
    connect(unmuteMemberAudioBtn, &QPushButton::clicked, this, &MainWindow::unmuteMemberAudio);
    connect(muteAllAudioBtn, &QPushButton::clicked, this, &MainWindow::muteAllAudio);

    return tab;
}

// 聊天室选项卡
QWidget* MainWindow::setupChatTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 聊天消息
    QGroupBox* chatGroup = new QGroupBox("聊天室");
    QVBoxLayout* chatLayout = new QVBoxLayout(chatGroup);

    chatHistory = new QTextEdit();
    chatHistory->setReadOnly(true);

    QHBoxLayout* messageLayout = new QHBoxLayout();
    messageEdit = new QLineEdit();
    QPushButton* sendMessageBtn = new QPushButton("发送");

    messageLayout->addWidget(messageEdit, 1);
    messageLayout->addWidget(sendMessageBtn);

    chatLayout->addWidget(chatHistory);
    chatLayout->addLayout(messageLayout);

    layout->addWidget(chatGroup);
    layout->addStretch();

    connect(sendMessageBtn, &QPushButton::clicked, this, &MainWindow::sendChatMessage);

    return tab;
}

// 邀请管理选项卡
QWidget* MainWindow::setupInviteTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 搜索账户
    QGroupBox* searchGroup = new QGroupBox("搜索账户");
    QFormLayout* searchLayout = new QFormLayout(searchGroup);

    searchPhoneEdit = new QLineEdit();
    QPushButton* searchByPhoneBtn = new QPushButton("通过手机号搜索");

    searchLayout->addRow("手机号:", searchPhoneEdit);
    searchLayout->addRow(searchByPhoneBtn);

    // 获取账户信息
    QGroupBox* accountInfoGroup = new QGroupBox("获取账户信息");
    QFormLayout* accountInfoLayout = new QFormLayout(accountInfoGroup);

    userUuidsEdit = new QLineEdit();
    userUuidsEdit->setPlaceholderText("输入用户UUID，多个用逗号分隔");
    QPushButton* getAccountInfoBtn = new QPushButton("获取账户信息");

    accountInfoLayout->addRow("用户UUID:", userUuidsEdit);
    accountInfoLayout->addRow(getAccountInfoBtn);

    // 搜索结果
    QGroupBox* resultsGroup = new QGroupBox("搜索结果");
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroup);

    searchResultsList = new QListWidget();

    resultsLayout->addWidget(searchResultsList);

    layout->addWidget(searchGroup);
    layout->addWidget(accountInfoGroup);
    layout->addWidget(resultsGroup);
    layout->addStretch();

    connect(searchByPhoneBtn, &QPushButton::clicked, this, &MainWindow::searchAccountByPhone);
    connect(getAccountInfoBtn, &QPushButton::clicked, this, &MainWindow::getAccountInfoList);

    return tab;
}

// 预会议管理选项卡
QWidget* MainWindow::setupPreMeetingTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    // 会议列表
    QGroupBox* meetingListGroup = new QGroupBox("会议列表");
    QVBoxLayout* meetingListLayout = new QVBoxLayout(meetingListGroup);

    meetingListTable = new QTableWidget(0, 5);
    meetingListTable->setHorizontalHeaderLabels({"会议ID", "会议号", "主题", "开始时间", "状态"});
    meetingListTable->horizontalHeader()->setStretchLastSection(true);
    meetingListTable->setMaximumHeight(200);  // 限制表格高度

    QPushButton* refreshMeetingListBtn = new QPushButton("刷新会议列表");

    meetingListLayout->addWidget(meetingListTable);
    meetingListLayout->addWidget(refreshMeetingListBtn);

    // 预定会议
    QGroupBox* scheduleGroup = new QGroupBox("预定会议");
    QFormLayout* scheduleLayout = new QFormLayout(scheduleGroup);

    scheduleSubjectEdit = new QLineEdit("预定会议测试");

    // 修正时间设置 - 使用 QDateTime 的正确方法
    QDateTime startTime = QDateTime::currentDateTime();
    startTime = startTime.addDays(1);
    scheduleStartTimeEdit = new QDateTimeEdit(startTime);
    scheduleStartTimeEdit->setCalendarPopup(true);

    QDateTime endTime = startTime;
    endTime = endTime.addSecs(3600);  // 添加1小时（3600秒）
    scheduleEndTimeEdit = new QDateTimeEdit(endTime);
    scheduleEndTimeEdit->setCalendarPopup(true);

    scheduleWaitingRoomCheck = new QCheckBox("启用等候室");
    scheduleRecordingCheck = new QCheckBox("启用云端录制");

    scheduleLayout->addRow("会议主题:", scheduleSubjectEdit);
    scheduleLayout->addRow("开始时间:", scheduleStartTimeEdit);
    scheduleLayout->addRow("结束时间:", scheduleEndTimeEdit);
    scheduleLayout->addRow(scheduleWaitingRoomCheck);
    scheduleLayout->addRow(scheduleRecordingCheck);

    QPushButton* scheduleBtn = new QPushButton("预定会议");
    scheduleLayout->addRow(scheduleBtn);

    // 编辑会议 - 新增详细编辑UI
    QGroupBox* editMeetingGroup = new QGroupBox("编辑会议");
    QFormLayout* editMeetingLayout = new QFormLayout(editMeetingGroup);

    editMeetingIdEdit = new QLineEdit();
    editMeetingIdEdit->setPlaceholderText("输入会议ID");

    editMeetingSubjectEdit = new QLineEdit();
    editMeetingSubjectEdit->setPlaceholderText("输入新主题（留空则不修改）");

    QDateTime editStartTime = QDateTime::currentDateTime();
    editStartTime = editStartTime.addDays(1);
    editMeetingStartTimeEdit = new QDateTimeEdit(editStartTime);
    editMeetingStartTimeEdit->setCalendarPopup(true);
    editMeetingStartTimeEdit->setEnabled(false);  // 默认不启用时间修改

    QDateTime editEndTime = editStartTime;
    editEndTime = editEndTime.addSecs(3600);
    editMeetingEndTimeEdit = new QDateTimeEdit(editEndTime);
    editMeetingEndTimeEdit->setCalendarPopup(true);
    editMeetingEndTimeEdit->setEnabled(false);  // 默认不启用时间修改

    QCheckBox* enableTimeEditCheck = new QCheckBox("修改会议时间");

    editMeetingWaitingRoomCheck = new QCheckBox("启用等候室");
    editMeetingRecordingCheck = new QCheckBox("启用云端录制");

    editMeetingMembersEdit = new QLineEdit();
    editMeetingMembersEdit->setPlaceholderText("输入成员UUID，多个用逗号分隔（留空则不修改）");

    editMeetingLayout->addRow("会议ID:", editMeetingIdEdit);
    editMeetingLayout->addRow("新主题:", editMeetingSubjectEdit);
    editMeetingLayout->addRow(enableTimeEditCheck);
    editMeetingLayout->addRow("开始时间:", editMeetingStartTimeEdit);
    editMeetingLayout->addRow("结束时间:", editMeetingEndTimeEdit);
    editMeetingLayout->addRow(editMeetingWaitingRoomCheck);
    editMeetingLayout->addRow(editMeetingRecordingCheck);
    editMeetingLayout->addRow("预定成员:", editMeetingMembersEdit);

    QPushButton* editMeetingBtn = new QPushButton("编辑会议");
    editMeetingLayout->addRow(editMeetingBtn);

    // 连接时间编辑复选框
    connect(enableTimeEditCheck, &QCheckBox::toggled, [this, enableTimeEditCheck]() {
        bool enabled = enableTimeEditCheck->isChecked();
        editMeetingStartTimeEdit->setEnabled(enabled);
        editMeetingEndTimeEdit->setEnabled(enabled);
    });

    // 会议操作
    QGroupBox* meetingOpsGroup = new QGroupBox("会议操作");
    QFormLayout* meetingOpsLayout = new QFormLayout(meetingOpsGroup);

    cancelMeetingIdEdit = new QLineEdit();
    cancelMeetingIdEdit->setPlaceholderText("输入会议ID");
    QPushButton* cancelMeetingBtn = new QPushButton("取消会议");

    getMeetingNumEdit = new QLineEdit();
    getMeetingNumEdit->setPlaceholderText("输入会议号");
    QPushButton* getPreMeetingInfoBtn = new QPushButton("获取会议信息");
    QPushButton* getScheduledMembersBtn = new QPushButton("获取预定成员");

    meetingOpsLayout->addRow("取消会议ID:", cancelMeetingIdEdit);
    meetingOpsLayout->addRow(cancelMeetingBtn);
    meetingOpsLayout->addRow("会议号:", getMeetingNumEdit);
    meetingOpsLayout->addRow(getPreMeetingInfoBtn);
    meetingOpsLayout->addRow(getScheduledMembersBtn);

    // 将所有组件添加到滚动布局中
    scrollLayout->addWidget(meetingListGroup);
    scrollLayout->addWidget(scheduleGroup);
    scrollLayout->addWidget(editMeetingGroup);
    scrollLayout->addWidget(meetingOpsGroup);
    scrollLayout->addStretch();

    // 设置滚动区域的内容
    scrollArea->setWidget(scrollWidget);

    // 将滚动区域添加到主布局
    layout->addWidget(scrollArea);

    connect(refreshMeetingListBtn, &QPushButton::clicked, this, &MainWindow::getMeetingList);
    connect(scheduleBtn, &QPushButton::clicked, this, &MainWindow::scheduleMeeting);
    connect(editMeetingBtn, &QPushButton::clicked, this, &MainWindow::editMeeting);
    connect(cancelMeetingBtn, &QPushButton::clicked, this, &MainWindow::cancelMeeting);
    connect(getPreMeetingInfoBtn, &QPushButton::clicked, this, &MainWindow::getPreMeetingInfo);
    connect(getScheduledMembersBtn, &QPushButton::clicked, this, &MainWindow::getScheduledMembers);

    return tab;
}

// 安全管理选项卡
QWidget* MainWindow::setupSecurityTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 权限设置
    QGroupBox* permissionGroup = new QGroupBox("聊天室权限设置");
    QFormLayout* permissionLayout = new QFormLayout(permissionGroup);

    chatroomPermCombo = new QComboBox();
    chatroomPermCombo->addItems({"1", "2", "3", "4"});
    waitingRoomChatCheck = new QCheckBox("允许等候室聊天");

    permissionLayout->addRow("聊天室权限:", chatroomPermCombo);
    permissionLayout->addRow(waitingRoomChatCheck);

    QHBoxLayout* permissionBtnLayout = new QHBoxLayout();
    QPushButton* getPermissionBtn = new QPushButton("获取权限");
    QPushButton* changePermissionBtn = new QPushButton("修改权限");

    permissionBtnLayout->addWidget(getPermissionBtn);
    permissionBtnLayout->addWidget(changePermissionBtn);

    permissionLayout->addRow(permissionBtnLayout);

    // 权限信息
    QGroupBox* infoGroup = new QGroupBox("权限信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

    securityInfoEdit = new QTextEdit();
    securityInfoEdit->setReadOnly(true);
    securityInfoEdit->setMaximumHeight(100);

    infoLayout->addWidget(securityInfoEdit);

    layout->addWidget(permissionGroup);
    layout->addWidget(infoGroup);
    layout->addStretch();

    connect(getPermissionBtn, &QPushButton::clicked, this, &MainWindow::getChatroomPermission);
    connect(changePermissionBtn, &QPushButton::clicked, this, &MainWindow::changeChatroomPermission);

    return tab;
}

// 等候室管理选项卡
QWidget* MainWindow::setupWaitingRoomTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 等候室信息
    QGroupBox* infoGroup = new QGroupBox("等候室信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

    waitingRoomInfoEdit = new QTextEdit();
    waitingRoomInfoEdit->setReadOnly(true);
    waitingRoomInfoEdit->setMaximumHeight(80);

    QPushButton* getInfoBtn = new QPushButton("获取等候室信息");

    infoLayout->addWidget(waitingRoomInfoEdit);
    infoLayout->addWidget(getInfoBtn);

    // 成员列表
    QGroupBox* memberGroup = new QGroupBox("等候室成员");
    QVBoxLayout* memberLayout = new QVBoxLayout(memberGroup);

    waitingRoomMemberTable = new QTableWidget(0, 3);
    waitingRoomMemberTable->setHorizontalHeaderLabels({"用户UUID", "用户名", "加入时间"});
    waitingRoomMemberTable->horizontalHeader()->setStretchLastSection(true);

    QPushButton* refreshMemberBtn = new QPushButton("刷新成员列表");

    memberLayout->addWidget(waitingRoomMemberTable);
    memberLayout->addWidget(refreshMemberBtn);

    // 成员操作
    QGroupBox* memberOpsGroup = new QGroupBox("成员操作");
    QFormLayout* memberOpsLayout = new QFormLayout(memberOpsGroup);

    waitingRoomUserUuidEdit = new QLineEdit();
    waitingRoomUserUuidEdit->setPlaceholderText("输入用户UUID");

    QHBoxLayout* singleMemberLayout = new QHBoxLayout();
    QPushButton* putInWaitingRoomBtn = new QPushButton("放入等候室");
    QPushButton* admitMemberBtn = new QPushButton("允许进入");
    QPushButton* expelMemberBtn = new QPushButton("移出会议");

    singleMemberLayout->addWidget(putInWaitingRoomBtn);
    singleMemberLayout->addWidget(admitMemberBtn);
    singleMemberLayout->addWidget(expelMemberBtn);

    QHBoxLayout* allMemberLayout = new QHBoxLayout();
    QPushButton* admitAllBtn = new QPushButton("允许所有人进入");
    QPushButton* expelAllBtn = new QPushButton("移出所有人");

    allMemberLayout->addWidget(admitAllBtn);
    allMemberLayout->addWidget(expelAllBtn);

    memberOpsLayout->addRow("用户UUID:", waitingRoomUserUuidEdit);
    memberOpsLayout->addRow(singleMemberLayout);
    memberOpsLayout->addRow(allMemberLayout);

    layout->addWidget(infoGroup);
    layout->addWidget(memberGroup);
    layout->addWidget(memberOpsGroup);
    layout->addStretch();

    connect(getInfoBtn, &QPushButton::clicked, this, &MainWindow::getWaitingRoomInfo);
    connect(refreshMemberBtn, &QPushButton::clicked, this, &MainWindow::getWaitingRoomMemberList);
    connect(putInWaitingRoomBtn, &QPushButton::clicked, this, &MainWindow::putInWaitingRoom);
    connect(admitMemberBtn, &QPushButton::clicked, this, &MainWindow::admitMember);
    connect(expelMemberBtn, &QPushButton::clicked, this, &MainWindow::expelMember);
    connect(admitAllBtn, &QPushButton::clicked, this, &MainWindow::admitAll);
    connect(expelAllBtn, &QPushButton::clicked, this, &MainWindow::expelAll);

    return tab;
}

// SDK初始化实现
void MainWindow::initializeSDK() {
    if (sdkInitialized) {
        logMessage("SDK已经初始化");
        return;
    }

    const char* sdkVersion = NETender_GetVersion();
    logMessage(QString("NETender SDK version: %1").arg(sdkVersion ? sdkVersion : "unknown"));

    QString appKey = appKeyEdit->text().trimmed();
    QString serverUrl = serverUrlEdit->text().trimmed();
    QString locale = localeCombo->currentText();

    // 输入验证
    if (appKey.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "App Key 不能为空");
        appKeyEdit->setFocus();
        return;
    }

    if (serverUrl.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "服务器地址不能为空");
        serverUrlEdit->setFocus();
        return;
    }

    // 验证URL格式
    if (!serverUrl.startsWith("http://") && !serverUrl.startsWith("https://")) {
        QMessageBox::warning(this, "输入错误", "服务器地址必须以 http:// 或 https:// 开头");
        serverUrlEdit->setFocus();
        return;
    }

    logMessage("开始初始化SDK...");
    logMessage(QString("配置参数:"));
    logMessage(QString("  App Key: %1").arg(appKey));
    logMessage(QString("  服务器: %1").arg(serverUrl));
    logMessage(QString("  语言: %1").arg(locale));

    // 转换为C字符串
    string sAppKey = appKey.toStdString();
    string sServerUrl = serverUrl.toStdString();
    string sLocale = locale.toStdString();

    NETenderInitConfig config = {};
    config.app_key = sAppKey.c_str();
    config.server_url = sServerUrl.c_str();
    config.locale = sLocale.c_str();
    QString logPath = getLogPath();
    std::string sLogPath = logPath.toStdString();
    logMessage(QString("  log_path: %1").arg(logPath));
    config.log_path = sLogPath.c_str();
    config.use_asset_server_config = -1;
    int32_t result = NETender_Initialize(&config, &sdkHandle);

    if (result == 0) {
        sdkInitialized = true;

        // 获取各个管理器
        meetingManager = NETender_GetMeetingManager(sdkHandle);
        accountManager = NETender_GetAccountManager(sdkHandle);
        audioManager = NETender_GetAudioManager(sdkHandle);
        videoManager = NETender_GetVideoManager(sdkHandle);
        memberManager = NETender_GetMemberManager(sdkHandle);
        screenShareManager = NETender_GetScreenShareManager(sdkHandle);
        chatroomManager = NETender_GetChatroomManager(sdkHandle);
        inviteManager = NETender_GetInviteManager(sdkHandle);
        preMeetingManager = NETender_GetPreMeetingManager(sdkHandle);
        securityManager = NETender_GetSecurityManager(sdkHandle);
        waitingRoomManager = NETender_GetWaitingRoomManager(sdkHandle);

        sdkVersion = NETender_GetVersion();
        logMessage(QString("SDK初始化成功，版本: %1").arg(sdkVersion ? sdkVersion : "unknown"));
        setupSDKCallbacks();
    } else {
        logMessage(QString("SDK初始化失败，错误代码: %1").arg(result));
    }
}

void MainWindow::uninitializeSDK() {
    if (!sdkInitialized) {
        logMessage("SDK未初始化");
        return;
    }

    unregisterSDKCallbacks();

    int32_t result = NETender_Uninitialize();

    if (result == 0) {
        sdkInitialized = false;
        sdkHandle = nullptr;
        meetingManager = nullptr;
        accountManager = nullptr;
        audioManager = nullptr;
        videoManager = nullptr;
        memberManager = nullptr;
        screenShareManager = nullptr;
        chatroomManager = nullptr;
        inviteManager = nullptr;
        preMeetingManager = nullptr;
        securityManager = nullptr;
        waitingRoomManager = nullptr;
        globalEventListenerRegistered = false;
        meetingListenerRegistered = false;
        messageChannelListenerRegistered = false;
        waitingRoomListenerRegistered = false;

        logMessage("SDK反初始化成功");
    } else {
        logMessage(QString("SDK反初始化失败，错误代码: %1").arg(result));
    }
}

// 账户管理实现
void MainWindow::loginByPassword() {
    if (!sdkInitialized || !accountManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        logMessage("用户名和密码不能为空");
        return;
    }

    NETenderLoginByPasswordOptions options = {};
    QByteArray usernameBytes = username.toUtf8();
    QByteArray passwordBytes = password.toUtf8();
    options.username = usernameBytes.constData();
    options.password = passwordBytes.constData();

    CallbackData* callbackData = new CallbackData{this, "密码登录", nullptr, nullptr};

    int32_t result = NETender_Account_LoginByPassword(accountManager, &options, &MainWindow::accountInfoCallback, callbackData);

    if (result != 0) {
        logMessage(QString("登录请求失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::loginByToken() {
    if (!sdkInitialized || !accountManager) {
        logMessage("请先初始化SDK");
        return;
    }

    NETenderLoginByTokenOptions options = {};
    QString accountId = accountIdEdit->text().trimmed();
    QString tokenUrl = tokenEdit->text().trimmed();
    QByteArray accountIdBytes = accountId.toUtf8();
    QByteArray tokenUrlBytes = tokenUrl.toUtf8();
    options.account_id = accountIdBytes.constData();
    options.account_token = tokenUrlBytes.constData();

    CallbackData* callbackData = new CallbackData{this, "Token登录", nullptr, nullptr};

    int32_t result = NETender_Account_LoginByToken(accountManager, &options, &MainWindow::accountInfoCallback, callbackData);

    if (result != 0) {
        logMessage(QString("Token登录请求失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::logout() {
    if (!sdkInitialized || !accountManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "退出登录", nullptr, nullptr};

    int32_t result = NETender_Account_Logout(accountManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("退出登录请求失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getAccountInfo() {
    if (!sdkInitialized || !accountManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "获取账户信息", nullptr, nullptr};

    int32_t result = NETender_Account_GetAccountInfo(accountManager, &MainWindow::accountInfoCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取账户信息失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::updateNickname() {
    if (!sdkInitialized || !accountManager) {
        logMessage("请先初始化SDK");
        return;
    }

    bool ok;
    QString newNickname = QInputDialog::getText(this, "更新昵称", "请输入新昵称:", QLineEdit::Normal, "", &ok);

    if (!ok || newNickname.isEmpty()) {
        return;
    }

    QByteArray nicknameBytes = newNickname.toUtf8();
    const char* newNicknameStr = nicknameBytes.constData();

    CallbackData* callbackData = new CallbackData{this, "更新昵称", nullptr, nullptr};

    int32_t result = NETender_Account_UpdateNickname(accountManager, newNicknameStr, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("更新昵称失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 会议管理实现
void MainWindow::createMeeting() {
    if (!sdkInitialized || !meetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString subject = meetingSubjectEdit->text().trimmed();
    if (subject.isEmpty()) {
        logMessage("会议主题不能为空");
        return;
    }

    NETenderMeetingCreateOptions options = {};
    QByteArray subjectBytes = subject.toUtf8();
    options.subject = subjectBytes.constData();

    // 设置新增参数
    QString password = meetingPasswordEdit->text().trimmed();
    if (!password.isEmpty()) {
        options.password = storeString(password.toStdString());
    }

    QString nickname = nickNameEdit->text().trimmed();
    if (!nickname.isEmpty()) {
        options.nickname = storeString(nickname.toStdString());
    }

    options.no_chat = noChatCheck->isChecked() ? 1 : -1;
    options.no_whiteboard = noWhiteboardCheck->isChecked() ? 1 : -1;
    options.no_captions = noCaptionsCheck->isChecked() ? 1 : -1;

    QString orgName = orgNameEdit->text().trimmed();
    if (!orgName.isEmpty()) {
        QByteArray orgNameBytes = orgName.toUtf8();
        options.org_name = orgNameBytes.constData();
    }

    QString avatarUrl = avatarUrlEdit->text().trimmed();
    if (!avatarUrl.isEmpty()) {
        QByteArray avatarUrlBytes = avatarUrl.toUtf8();
        options.avatar = avatarUrlBytes.constData();
    }

    options.join_timeout = joinTimeoutSpin->value();

    QString session = sessionEdit->text().trimmed();
    if (!session.isEmpty()) {
        QByteArray sessionBytes = session.toUtf8();
        options.session = sessionBytes.constData();
    }

    options.sub_type = meetingSubTypeCombo->currentData().toInt();
    options.start_time = -1;
    options.end_time = -1;
    options.enable_waiting_room = waitingRoomCheck->isChecked() ? 1 : 0;

    // 设置云端录制配置
    NETenderCloudRecordConfig recordConfig = {};
    recordConfig.enable = recordingCheck->isChecked() ? 1 : 0;
    options.cloud_record_config = &recordConfig;

    CallbackData* callbackData = new CallbackData{this, "创建会议", nullptr, nullptr};

    int32_t result = NETender_Meeting_CreateMeeting(meetingManager, &options, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("创建会议失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("创建会议请求已发送...");
    }
}

void MainWindow::joinMeeting() {
    if (!sdkInitialized || !meetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString meetingNum = meetingNumEdit->text().trimmed();
    QString userUuid = joinUserUuidEdit->text().trimmed();
    QString nickname = joinNicknameEdit->text().trimmed();

    if (meetingNum.isEmpty()) {
        logMessage("会议号不能为空");
        return;
    }

    if (userUuid.isEmpty()) {
        logMessage("用户UUID不能为空");
        return;
    }

    if (nickname.isEmpty()) {
        logMessage("昵称不能为空");
        return;
    }

    NETenderMeetingJoinOptions options = {};
    QByteArray meetingNumBytes = meetingNum.toUtf8();
    QByteArray userUuidBytes = userUuid.toUtf8();
    QByteArray nicknameBytes = nickname.toUtf8();

    options.meeting_num = meetingNumBytes.constData();
    options.user_uuid = userUuidBytes.constData();
    options.nickname = nicknameBytes.constData();
    CallbackData* callbackData = new CallbackData{this, "加入会议", nullptr, nullptr};

    int32_t result = NETender_Meeting_JoinMeeting(meetingManager, &options, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("加入会议失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("加入会议请求已发送...");
    }
}

void MainWindow::leaveMeeting() {
    if (!sdkInitialized || !meetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "离开会议", nullptr, nullptr};

    int32_t result = NETender_Meeting_LeaveMeeting(meetingManager, 0, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("离开会议失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::endMeeting() {
    if (!sdkInitialized || !meetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "结束会议", nullptr, nullptr};

    int32_t result = NETender_Meeting_EndMeeting(meetingManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("结束会议失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getMeetingInfo() {
    if (!sdkInitialized || !meetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    NETenderMeetingSDKInfo* info = nullptr;
    int32_t result = NETender_Meeting_GetMeetingInfo(meetingManager, &info);

    if (result == 0 && info) {
        logMessage(QString("获取会议信息成功 - 主题: %1, 会议号: %2")
                .arg(info->subject ? info->subject : "未知")
                .arg(info->meeting_num ? info->meeting_num : "未知"));
        NETender_FreeMeetingInfo(info);
    } else {
        logMessage(QString("获取会议信息失败，错误代码: %1").arg(result));
    }
}

// 音频管理实现
void MainWindow::refreshSpeakerDevices() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "刷新扬声器设备", nullptr, nullptr};

    int32_t result = NETender_Audio_GetSpeakerDeviceList(audioManager, &MainWindow::deviceListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("刷新扬声器设备失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::refreshMicrophoneDevices() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "刷新麦克风设备", nullptr, nullptr};

    int32_t result = NETender_Audio_GetMicrophoneDeviceList(audioManager, &MainWindow::deviceListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("刷新麦克风设备失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::setSpeakerDevice() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (speakerCombo->currentIndex() < 0) {
        logMessage("请先选择扬声器设备");
        return;
    }

    QString deviceId = speakerCombo->currentData().toString();
    CallbackData* callbackData = new CallbackData{this, "设置扬声器", nullptr, nullptr};

    int32_t result = NETender_Audio_SetSpeakerDevice(audioManager, deviceId.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置扬声器失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::setMicrophoneDevice() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (microphoneCombo->currentIndex() < 0) {
        logMessage("请先选择麦克风设备");
        return;
    }

    QString deviceId = microphoneCombo->currentData().toString();
    CallbackData* callbackData = new CallbackData{this, "设置麦克风", nullptr, nullptr};

    int32_t result = NETender_Audio_SetMicrophoneDevice(audioManager, deviceId.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置麦克风失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::setSpeakerVolume() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    uint32_t volume = speakerSlider->value();
    CallbackData* callbackData = new CallbackData{this, "设置扬声器音量", nullptr, nullptr};

    int32_t result = NETender_Audio_SetSpeakerVolume(audioManager, volume, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置扬声器音量失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::setMicrophoneVolume() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    uint32_t volume = micSlider->value();
    CallbackData* callbackData = new CallbackData{this, "设置麦克风音量", nullptr, nullptr};

    int32_t result = NETender_Audio_SetMicrophoneVolume(audioManager, volume, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置麦克风音量失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::muteMyAudio() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "静音音频", nullptr, nullptr};

    int32_t result = NETender_Audio_MuteMyAudio(audioManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("静音音频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::unmuteMyAudio() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "取消静音", nullptr, nullptr};
    int32_t result = NETender_Audio_UnmuteMyAudio(audioManager, &MainWindow::asyncCallback, callbackData);

    if (result == 0) {
        logMessage("取消音频静音成功");
    } else {
        logMessage(QString("取消音频静音失败，错误代码: %1").arg(result));
    }
}

void MainWindow::startSpeakerTest() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "开始扬声器测试", nullptr, nullptr};

    int32_t result = NETender_Audio_StartSpeakerDeviceTest(audioManager, "test_audio_file.wav", &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开始扬声器测试失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::stopSpeakerTest() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "停止扬声器测试", nullptr, nullptr};

    int32_t result = NETender_Audio_StopSpeakerDeviceTest(audioManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("停止扬声器测试失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::startMicrophoneTest() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "开始麦克风测试", nullptr, nullptr};

    int32_t result = NETender_Audio_StartMicrophoneDeviceTest(audioManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开始麦克风测试失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::stopMicrophoneTest() {
    if (!sdkInitialized || !audioManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "停止麦克风测试", nullptr, nullptr};

    int32_t result = NETender_Audio_StopMicrophoneDeviceTest(audioManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("停止麦克风测试失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 视频管理实现
void MainWindow::refreshVideoDevices() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "刷新视频设备", nullptr, nullptr};

    int32_t result = NETender_Video_GetCameraDeviceList(videoManager, &MainWindow::deviceListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("刷新视频设备失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::setCameraDevice() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (cameraCombo->currentIndex() < 0) {
        logMessage("请先选择摄像头设备");
        return;
    }

    QString deviceId = cameraCombo->currentData().toString();
    CallbackData* callbackData = new CallbackData{this, "设置摄像头", nullptr, nullptr};

    int32_t result = NETender_Video_SetCameraDevice(videoManager, deviceId.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置摄像头失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::startVideoPreview() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "开始视频预览", nullptr, nullptr};

    int32_t result = NETender_Video_StartVideoPreview(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开始视频预览失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::stopVideoPreview() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "停止视频预览", nullptr, nullptr};

    int32_t result = NETender_Video_StopVideoPreview(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("停止视频预览失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::muteMyVideo() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "关闭视频", nullptr, nullptr};

    int32_t result = NETender_Video_MuteMyVideo(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("关闭视频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::unmuteMyVideo() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "开启视频", nullptr, nullptr};

    int32_t result = NETender_Video_UnmuteMyVideo(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开启视频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 设置本地视频画布
void MainWindow::setupLocalVideoCanvas() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (!localVideoWidget) {
        logMessage("本地视频窗口未创建");
        return;
    }

    void* videoHwnd = localVideoWidget->GetVideoHwnd();
    if (!videoHwnd) {
        logMessage("获取本地视频窗口句柄失败");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置本地视频画布", nullptr, nullptr};

    int32_t result = NETender_Video_SetupLocalVideoCanvas(videoManager, videoHwnd, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置本地视频画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("正在设置本地视频画布...");
    }
}

// 移除本地视频画布
void MainWindow::removeLocalVideoCanvas() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "移除本地视频画布", nullptr, nullptr};

    //    int32_t result = NETender_Video_RemoveLocalVideoCanvas(
    //        videoManager, &MainWindow::asyncCallback, callbackData
    //    );
    int32_t result = NETender_Video_SetupLocalVideoCanvas(videoManager, nullptr, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("移除本地视频画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("正在移除本地视频画布...");
    }
}

// 设置视频预览画布
void MainWindow::setupVideoPreviewCanvas() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (!previewVideoWidget) {
        logMessage("预览视频窗口未创建");
        return;
    }

    void* videoHwnd = previewVideoWidget->GetVideoHwnd();
    if (!videoHwnd) {
        logMessage("获取预览视频窗口句柄失败");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置视频预览画布", nullptr, nullptr};

    int32_t result = NETender_Video_SetupVideoPreviewCanvas(videoManager, videoHwnd, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置视频预览画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("正在设置视频预览画布...");
    }
}

// 设置远程视频画布
void MainWindow::setupRemoteVideoCanvas(const QString& userId) {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (!remoteVideoWidget) {
        logMessage("远程视频窗口未创建");
        return;
    }

    void* videoHwnd = remoteVideoWidget->GetVideoHwnd();
    if (!videoHwnd) {
        logMessage("获取远程视频窗口句柄失败");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置远程视频画布", nullptr, nullptr};

    int32_t result =
        NETender_Video_SetupRemoteVideoCanvas(videoManager, userId.toUtf8().constData(), videoHwnd, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置远程视频画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在为用户 %1 设置远程视频画布...").arg(userId));

        // 设置画布后自动订阅主流
        CallbackData* subscribeCallbackData = new CallbackData{this, "自动订阅远程视频主流", nullptr, nullptr};
        NETender_Video_SubscribeRemoteVideoStream(videoManager, userId.toUtf8().constData(),
            0,  // 主流
            &MainWindow::asyncCallback, subscribeCallbackData);
    }
}

void MainWindow::removeRemoteVideoCanvas(const QString& userId) {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置远程视频画布", nullptr, nullptr};

    int32_t result =
        NETender_Video_SetupRemoteVideoCanvas(videoManager, userId.toUtf8().constData(), nullptr, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置远程视频画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在为用户 %1 设置远程视频画布...").arg(userId));

        // 设置画布后自动订阅主流
        CallbackData* subscribeCallbackData = new CallbackData{this, "自动订阅远程视频主流", nullptr, nullptr};
        NETender_Video_SubscribeRemoteVideoStream(videoManager, userId.toUtf8().constData(),
            0,  // 主流
            &MainWindow::asyncCallback, subscribeCallbackData);
    }
}

// 设置远程视频辅流画布
void MainWindow::setupRemoteVideoSubStreamCanvas(const QString& userId) {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (!subStreamVideoWidget) {
        logMessage("辅流视频窗口未创建");
        return;
    }

    void* videoHwnd = subStreamVideoWidget->GetVideoHwnd();
    if (!videoHwnd) {
        logMessage("获取辅流视频窗口句柄失败");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置远程视频辅流画布", nullptr, nullptr};

    int32_t result = NETender_Video_SetupRemoteVideoSubStreamCanvas(
        videoManager, userId.toUtf8().constData(), videoHwnd, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置远程视频辅流画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在为用户 %1 设置远程视频辅流画布...").arg(userId));
    }
}

// 设置远程视频辅流画布
void MainWindow::removeRemoteVideoSubStreamCanvas(const QString& userId) {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "设置远程视频辅流画布", nullptr, nullptr};

    int32_t result =
        NETender_Video_SetupRemoteVideoSubStreamCanvas(videoManager, userId.toUtf8().constData(), nullptr, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("设置远程视频辅流画布失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在为用户 %1 设置远程视频辅流画布...").arg(userId));
    }
}

// 视频配置实现
void MainWindow::setVideoProfile() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    int resolution = videoResolutionCombo->currentData().toInt();
    int frameRate = frameRateSpinBox->value();

    int32_t result = NETender_Video_SetVideoProfile(videoManager, resolution, frameRate);

    if (result == 0) {
        logMessage(QString("设置视频配置成功 - 分辨率: %1, 帧率: %2fps").arg(videoResolutionCombo->currentText()).arg(frameRate));
    } else {
        logMessage(QString("设置视频配置失败，错误代码: %1").arg(result));
    }
}

// 视频控制实现
void MainWindow::unmuteMyVideoEx() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "开启视频", nullptr, nullptr};

    int32_t result = NETender_Video_UnmuteMyVideo(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开启视频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::muteMyVideoEx() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "关闭视频", nullptr, nullptr};

    int32_t result = NETender_Video_MuteMyVideo(videoManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("关闭视频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 远程视频流订阅实现
void MainWindow::subscribeRemoteVideoStream() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userId = subscribeUserIdEdit->text().trimmed();
    if (userId.isEmpty()) {
        logMessage("用户ID不能为空");
        subscribeUserIdEdit->setFocus();
        return;
    }

    int streamType = streamTypeCombo->currentData().toInt();

    CallbackData* callbackData = new CallbackData{this, "订阅远程视频流", nullptr, nullptr};

    int32_t result =
        NETender_Video_SubscribeRemoteVideoStream(videoManager, userId.toUtf8().constData(), streamType, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("订阅远程视频流失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在订阅用户 %1 的%2视频流...").arg(userId).arg(streamType == 0 ? "主流" : "辅流"));
    }
}

void MainWindow::unsubscribeRemoteVideoStream() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userId = unsubscribeUserIdEdit->text().trimmed();
    if (userId.isEmpty()) {
        logMessage("用户ID不能为空");
        unsubscribeUserIdEdit->setFocus();
        return;
    }

    int streamType = streamTypeCombo->currentData().toInt();

    CallbackData* callbackData = new CallbackData{this, "取消订阅远程视频流", nullptr, nullptr};

    int32_t result =
        NETender_Video_UnsubscribeRemoteVideoStream(videoManager, userId.toUtf8().constData(), streamType, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("取消订阅远程视频流失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在取消订阅用户 %1 的%2视频流...").arg(userId).arg(streamType == 0 ? "主流" : "辅流"));
    }
}

void MainWindow::subscribeRemoteVideoSubStream() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userId = subscribeSubStreamUserIdEdit->text().trimmed();
    if (userId.isEmpty()) {
        logMessage("用户ID不能为空");
        subscribeSubStreamUserIdEdit->setFocus();
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "订阅远程视频辅流", nullptr, nullptr};

    int32_t result =
        NETender_Video_SubscribeRemoteVideoSubStream(videoManager, userId.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("订阅远程视频辅流失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在订阅用户 %1 的远程视频辅流...").arg(userId));
    }
}

void MainWindow::unsubscribeRemoteVideoSubStream() {
    if (!sdkInitialized || !videoManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userId = unsubscribeSubStreamUserIdEdit->text().trimmed();
    if (userId.isEmpty()) {
        logMessage("用户ID不能为空");
        unsubscribeSubStreamUserIdEdit->setFocus();
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "取消订阅远程视频辅流", nullptr, nullptr};

    int32_t result =
        NETender_Video_UnsubscribeRemoteVideoSubStream(videoManager, userId.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("取消订阅远程视频辅流失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage(QString("正在取消订阅用户 %1 的远程视频辅流...").arg(userId));
    }
}

// 屏幕共享实现
void MainWindow::refreshScreenSources() {
    if (!sdkInitialized || !screenShareManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "刷新屏幕共享源", nullptr, nullptr};

    int32_t result = NETender_ScreenShare_GetScreenCaptureSourceList(screenShareManager, &MainWindow::screenSourceCallback, callbackData);

    if (result != 0) {
        logMessage(QString("刷新屏幕共享源失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::startScreenShare() {
    if (!sdkInitialized || !screenShareManager) {
        logMessage("请先初始化SDK");
        return;
    }

    if (screenSourceList->currentRow() < 0) {
        logMessage("请先选择共享源");
        return;
    }

    // 这里需要根据实际SDK实现来获取选中的源ID
    // 假设我们从列表项的data中获取source_id
    QListWidgetItem* currentItem = screenSourceList->currentItem();

    if (!currentItem) {
        logMessage("请先选择共享源");
        return;
    }

    QString sourceIdStr = currentItem->data(Qt::UserRole).toString();
    bool ok;
    int64_t sourceId = sourceIdStr.toLongLong(&ok);

    NETenderScreenShareConfig config = {};
    // config.source_id = sourceId;
    config.source_id = sourceIdStr.toStdString().c_str();
    config.prefer_motion = 1;
    config.is_linux = -1;
    // config.enable_system_audio = systemAudioCheck->isChecked() ? 1 : 0;

    CallbackData* callbackData = new CallbackData{this, "开始屏幕共享", nullptr, nullptr};

    int32_t result = NETender_ScreenShare_StartScreenShare(screenShareManager, &config, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("开始屏幕共享失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::stopScreenShare() {
    if (!sdkInitialized || !screenShareManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "停止屏幕共享", nullptr, nullptr};
    NETenderStopScreenShareConfig config;
    config.need_request_meeting_server_screen_share = 1;
    int32_t result = NETender_ScreenShare_StopScreenShare(screenShareManager, &config, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("停止屏幕共享失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 聊天室实现
void MainWindow::sendChatMessage() {
    if (!sdkInitialized || !chatroomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString messageText = messageEdit->text();
    if (messageText.isEmpty()) {
        logMessage("消息内容不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "发送聊天消息", nullptr, nullptr};
    NETenderChatroomTextMessageOptions options;
    options.id_client = GetRandomClientIdCStr();
    options.content = messageText.toStdString().c_str();

    QString uuidsText = userUuidsEdit->text().trimmed();
    QStringList uuidList = uuidsText.split(',', Qt::SkipEmptyParts);
    std::vector<const char*> c_uuids;

    for (const QString& uuid : uuidList) {
        QString trimmedUuid = uuid.trimmed();

        if (!trimmedUuid.isEmpty()) {
            c_uuids.push_back(trimmedUuid.toStdString().c_str());
        }
    }

    options.to_user_uuid_list = c_uuids.data();
    options.to_user_uuid_count = static_cast<int32_t>(c_uuids.size());
    int32_t result = NETender_Chatroom_SendTextMessage(chatroomManager, &options, &MainWindow::chatMessageCallback, callbackData);

    if (result != 0) {
        logMessage(QString("发送聊天消息失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        messageEdit->clear();
    }
}

// 成员管理实现
void MainWindow::refreshMemberList() {
    if (!sdkInitialized || !memberManager) {
        logMessage("请先初始化SDK");
        return;
    }

    NETenderMember* members = nullptr;
    uint32_t count = 0;
    int32_t result = NETender_Member_GetMemberList(memberManager, &members, &count);

    if (result == 0) {
        logMessage(QString("获取成员列表成功，共 %1 个成员").arg(count));

        // 更新成员表格
        memberTable->setRowCount(count);

        for (uint32_t i = 0; i < count; ++i) {
            memberTable->setItem(i, 0, new QTableWidgetItem(members[i].user_uuid ? members[i].user_uuid : ""));
            memberTable->setItem(i, 1, new QTableWidgetItem(members[i].user_name ? members[i].user_name : ""));
            memberTable->setItem(i, 2, new QTableWidgetItem(members[i].is_host ? "主持人" : "成员"));
            memberTable->setItem(i, 3, new QTableWidgetItem(members[i].is_audio_on ? "开启" : "关闭"));
            memberTable->setItem(i, 4, new QTableWidgetItem(members[i].is_video_on ? "开启" : "关闭"));
        }

        NETender_FreeMemberList(members, count);
    } else {
        logMessage(QString("获取成员列表失败，错误代码: %1").arg(result));
    }
}

void MainWindow::muteMemberAudio() {
    if (!sdkInitialized || !memberManager) {
        logMessage("请先初始化SDK");
        return;
    }

    int currentRow = memberTable->currentRow();

    if (currentRow < 0) {
        logMessage("请先选择要静音的成员");
        return;
    }

    QString userUuid = memberTable->item(currentRow, 0)->text();
    CallbackData* callbackData = new CallbackData{this, "静音成员音频", nullptr, nullptr};

    int32_t result = NETender_Member_MuteMemberAudio(memberManager, userUuid.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("静音成员音频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::unmuteMemberAudio() {
    if (!sdkInitialized || !memberManager) {
        logMessage("请先初始化SDK");
        return;
    }

    int currentRow = memberTable->currentRow();

    if (currentRow < 0) {
        logMessage("请先选择要取消静音的成员");
        return;
    }

    QString userUuid = memberTable->item(currentRow, 0)->text();
    CallbackData* callbackData = new CallbackData{this, "取消静音成员音频", nullptr, nullptr};

    int32_t result = NETender_Member_UnmuteMemberAudio(memberManager, userUuid.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("取消静音成员音频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::muteAllAudio() {
    if (!sdkInitialized || !memberManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "静音所有成员音频", nullptr, nullptr};

    int32_t result = NETender_Member_MuteAllAudio(memberManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("静音所有成员音频失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 邀请管理实现
void MainWindow::searchAccountByPhone() {
    if (!sdkInitialized || !inviteManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString phone = searchPhoneEdit->text().trimmed();

    if (phone.isEmpty()) {
        logMessage("手机号不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "通过手机号搜索账户", nullptr, nullptr};

    int32_t result = NETender_Invite_SearchAccountByPhone(inviteManager, phone.toUtf8().constData(), &MainWindow::accountListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("搜索账户失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getAccountInfoList() {
    if (!sdkInitialized || !inviteManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString uuidsText = userUuidsEdit->text().trimmed();

    if (uuidsText.isEmpty()) {
        logMessage("用户UUID不能为空");
        return;
    }

    QStringList uuidList = uuidsText.split(',', Qt::SkipEmptyParts);
    std::vector<const char*> c_uuids;

    for (const QString& uuid : uuidList) {
        QString trimmedUuid = uuid.trimmed();

        if (!trimmedUuid.isEmpty()) {
            c_uuids.push_back(trimmedUuid.toStdString().c_str());
        }
    }

    CallbackData* callbackData = new CallbackData{this, "获取账户信息列表", nullptr, nullptr};

    int32_t result = NETender_Invite_GetAccountInfoList(
        inviteManager, c_uuids.data(), static_cast<int32_t>(c_uuids.size()), &MainWindow::accountListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取账户信息列表失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 预会议管理实现
void MainWindow::getMeetingList() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    NETenderMeetingListParams params = {};
    params.offset = -1;
    params.size = -1;

    CallbackData* callbackData = new CallbackData{this, "获取会议列表", nullptr, nullptr};

    int32_t result = NETender_PreMeeting_GetMeetingListBySize(preMeetingManager, &params, &MainWindow::meetingListCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取会议列表失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::scheduleMeeting() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString subject = scheduleSubjectEdit->text().trimmed();

    if (subject.isEmpty()) {
        logMessage("会议主题不能为空");
        return;
    }

    NETenderPreMeetingCreateOptions options = {};
    options.subject = subject.toStdString().c_str();
    options.start_time = scheduleStartTimeEdit->dateTime().toMSecsSinceEpoch();
    options.end_time = scheduleEndTimeEdit->dateTime().toMSecsSinceEpoch();
    options.enable_waiting_room = scheduleWaitingRoomCheck->isChecked() ? 1 : -1;

    // 设置云端录制配置
    NETenderPreMeetingCloudRecordConfig recordConfig = {};
    recordConfig.enable = scheduleRecordingCheck->isChecked() ? 1 : 0;
    options.cloud_record_config = &recordConfig;

    CallbackData* callbackData = new CallbackData{this, "预定会议", nullptr, nullptr};

    int32_t result = NETender_PreMeeting_ScheduleMeeting(preMeetingManager, &options, &MainWindow::meetingInfoCallback, callbackData);

    if (result != 0) {
        logMessage(QString("预定会议失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::editMeeting() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString meetingIdText = editMeetingIdEdit->text().trimmed();

    if (meetingIdText.isEmpty()) {
        logMessage("会议ID不能为空");
        return;
    }

    bool ok;
    int64_t meetingId = meetingIdText.toLongLong(&ok);

    if (!ok) {
        logMessage("会议ID格式错误");
        return;
    }

    NETenderPreMeetingEditOptions options = {};

    // 设置主题（如果不为空）
    QString subject = editMeetingSubjectEdit->text().trimmed();
    if (!subject.isEmpty()) {
        options.subject = storeString(subject.toStdString());
    } else {
        options.subject = nullptr;  // 不更新主题
    }

    // 设置开始和结束时间（如果启用了时间编辑）
    if (editMeetingStartTimeEdit->isEnabled()) {
        options.start_time = editMeetingStartTimeEdit->dateTime().toMSecsSinceEpoch();
        options.end_time = editMeetingEndTimeEdit->dateTime().toMSecsSinceEpoch();
    } else {
        options.start_time = -1;  // 不更新开始时间
        options.end_time = -1;    // 不更新结束时间
    }

    // 设置等候室选项
    options.enable_waiting_room = editMeetingWaitingRoomCheck->isChecked() ? 1 : 0;

    // 设置预定成员（如果不为空）
    QString membersText = editMeetingMembersEdit->text().trimmed();
    if (!membersText.isEmpty()) {
        QStringList uuidList = membersText.split(',', Qt::SkipEmptyParts);
        std::vector<NETenderScheduledMember> members;

        for (const QString& uuid : uuidList) {
            QString trimmedUuid = uuid.trimmed();
            if (!trimmedUuid.isEmpty()) {
                NETenderScheduledMember member = {};
                member.user_uuid = storeString(trimmedUuid.toStdString());
                members.push_back(member);
            }
        }

        if (!members.empty()) {
            options.scheduled_members = members.data();
            options.scheduled_member_count = static_cast<int32_t>(members.size());
        } else {
            options.scheduled_members = nullptr;
            options.scheduled_member_count = -1;
        }
    } else {
        options.scheduled_members = nullptr;
        options.scheduled_member_count = -1;
    }

    // 设置云端录制配置
    NETenderPreMeetingCloudRecordConfig recordConfig = {};
    recordConfig.enable = editMeetingRecordingCheck->isChecked() ? 1 : 0;
    options.cloud_record_config = &recordConfig;

    CallbackData* callbackData = new CallbackData{this, "编辑会议", nullptr, nullptr};

    int32_t result = NETender_PreMeeting_EditMeeting(preMeetingManager, meetingId, &options, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("编辑会议失败，错误代码: %1").arg(result));
        delete callbackData;
    } else {
        logMessage("编辑会议请求已发送...");
    }
}

void MainWindow::cancelMeeting() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString meetingIdText = cancelMeetingIdEdit->text().trimmed();

    if (meetingIdText.isEmpty()) {
        logMessage("会议ID不能为空");
        return;
    }

    bool ok;
    int64_t meetingId = meetingIdText.toLongLong(&ok);

    if (!ok) {
        logMessage("会议ID格式错误");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "取消会议", nullptr, nullptr};

    int32_t result = NETender_PreMeeting_CancelMeeting(preMeetingManager, meetingId, 0, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("取消会议失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getPreMeetingInfo() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString meetingNum = getMeetingNumEdit->text().trimmed();

    if (meetingNum.isEmpty()) {
        logMessage("会议号不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "获取预会议信息", nullptr, nullptr};

    int32_t result =
    NETender_PreMeeting_GetMeetingInfoByMeetingNum(preMeetingManager, meetingNum.toUtf8().constData(), &MainWindow::meetingInfoCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取预会议信息失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getScheduledMembers() {
    if (!sdkInitialized || !preMeetingManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString meetingNum = getMeetingNumEdit->text().trimmed();

    if (meetingNum.isEmpty()) {
        logMessage("会议号不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "获取预定成员", nullptr, nullptr};

    int32_t result = NETender_PreMeeting_GetScheduledMembers(
        preMeetingManager, meetingNum.toUtf8().constData(), &MainWindow::scheduledMemberCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取预定成员失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 安全管理实现
void MainWindow::getChatroomPermission() {
    if (!sdkInitialized || !securityManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "获取聊天室权限", nullptr, nullptr};

    int32_t result = NETender_Security_GetChatroomPermission(securityManager, &MainWindow::chatroomPermissionCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取聊天室权限失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::changeChatroomPermission() {
    if (!sdkInitialized || !securityManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString crPerm = chatroomPermCombo->currentText();
    QString wtPrChat = waitingRoomChatCheck->isChecked() ? "1" : "0";

    CallbackData* callbackData = new CallbackData{this, "修改聊天室权限", nullptr, nullptr};

    int32_t result = NETender_Security_ChangeChatroomPermission(
        securityManager, crPerm.toUtf8().constData(), wtPrChat.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("修改聊天室权限失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

// 等候室管理实现
void MainWindow::getWaitingRoomMemberList() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "获取等候室成员列表", nullptr, nullptr};

    int32_t result = NETender_WaitingRoom_GetMemberList(waitingRoomManager, 0, 50, 1, &MainWindow::waitingRoomMemberCallback, callbackData);

    if (result != 0) {
        logMessage(QString("获取等候室成员列表失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::getWaitingRoomInfo() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    NETenderWaitingRoomInfo info = {};
    int32_t result = NETender_WaitingRoom_GetInfo(waitingRoomManager, &info);

    if (result == 0) {
        QString infoText = QString("等候室状态: %1\n成员数量: %2").arg(info.is_enabled ? "已启用" : "未启用").arg(info.member_count);
        waitingRoomInfoEdit->setPlainText(infoText);
        logMessage("获取等候室信息成功");
    } else {
        logMessage(QString("获取等候室信息失败，错误代码: %1").arg(result));
    }
}

void MainWindow::putInWaitingRoom() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userUuid = waitingRoomUserUuidEdit->text().trimmed();

    if (userUuid.isEmpty()) {
        logMessage("用户UUID不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "放入等候室", nullptr, nullptr};

    int32_t result =
        NETender_WaitingRoom_PutInWaitingRoom(waitingRoomManager, userUuid.toUtf8().constData(), &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("放入等候室失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::admitMember() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userUuid = waitingRoomUserUuidEdit->text().trimmed();

    if (userUuid.isEmpty()) {
        logMessage("用户UUID不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "允许成员进入", nullptr, nullptr};

    int32_t result = NETender_WaitingRoom_AdmitMember(waitingRoomManager, userUuid.toUtf8().constData(), 0, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("允许成员进入失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::expelMember() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    QString userUuid = waitingRoomUserUuidEdit->text().trimmed();

    if (userUuid.isEmpty()) {
        logMessage("用户UUID不能为空");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "移出成员", nullptr, nullptr};

    int32_t result = NETender_WaitingRoom_ExpelMember(waitingRoomManager, userUuid.toUtf8().constData(), 0, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("移出成员失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::admitAll() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "允许所有人进入", nullptr, nullptr};

    int32_t result = NETender_WaitingRoom_AdmitAll(waitingRoomManager, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("允许所有人进入失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}

void MainWindow::expelAll() {
    if (!sdkInitialized || !waitingRoomManager) {
        logMessage("请先初始化SDK");
        return;
    }

    CallbackData* callbackData = new CallbackData{this, "移出所有人", nullptr, nullptr};

    int32_t result = NETender_WaitingRoom_ExpelAll(waitingRoomManager, 0, &MainWindow::asyncCallback, callbackData);

    if (result != 0) {
        logMessage(QString("移出所有人失败，错误代码: %1").arg(result));
        delete callbackData;
    }
}
