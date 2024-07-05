#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>
#include <functional>

#include <QFile>
#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QVariantMap>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QAtomicInteger>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>

#include "config.h"
#include "macro_qjson.h"
#include "network_runner.h"
#include "notification_delegate.h"

// =================================================================================================
//         LOGGER
// =================================================================================================

template <typename T>
class __Log {
public:
    static QVector<QString> Logs;

    static void AddLog(const QString &log) {
        Logs.push_back(log);
        SEND_NOTIFY({ 0, MessageType::LogMessage });
        // std::cout << log << '\n';
    }

    static void FatalLog(const QString &log, int exitCode) {
        // Logs.push_back(log);
        // std::cout << log << '\n';
        QMessageBox(QMessageBox::Icon::Critical, "ERROR", log, QMessageBox::StandardButton::Abort).exec();
        exit(exitCode);
    }

    static bool HasLog() { return !Logs.empty(); }

    static QString PopLog() {
        if (Logs.empty()) return {};
        QString front = Logs.front();
        Logs.pop_front();
        return front;
    }

    static QVector<QString>::iterator IterLog() { return Logs.begin(); }
    static QVector<QString>::iterator IterLogEnd() { return Logs.end(); }
};

template <typename T> QVector<QString> __Log<T>::Logs;

typedef __Log<int> Log;

// =================================================================================================
//         CONFIG
// =================================================================================================

static void MergeJsonObjects(QJsonObject &target, const QJsonObject &source) {
    for (const QString &key : source.keys()) {
        if (target.contains(key)) {
            if (target[key].isObject() && source[key].isObject()) {
                QJsonObject targetSubObj = target[key].toObject();
                QJsonObject sourceSubObj = source[key].toObject();
                MergeJsonObjects(targetSubObj, sourceSubObj);
                target[key] = targetSubObj;
            } else if (target[key].isArray() && source[key].isArray()) {
                QJsonArray targetArray = target[key].toArray();
                QJsonArray sourceArray = source[key].toArray();
                for (const QJsonValue &value : sourceArray) {
                    targetArray.append(value);
                }
                target[key] = targetArray;
            } else {
                target.insert(key, source.value(key));
            }
        } else {
            target.insert(key, source.value(key));
        }
    }
}

typedef QVariantMap ServerVariablesPreset;

#define PORT_TYPE uint16_t
#define INT_TYPE int32_t
#define STR_TYPE QString

enum MapTags: uint64_t {
    Small          = (1u << 0),
    Medium         = (1u << 1),
    Large          = (1u << 2),
    VeryLarge      = (1u << 3),
    Conquest       = (1u << 4),
    Breakthrough   = (1u << 5),
    CloseQuarter   = (1u << 6),
    Destruction    = (1u << 7),
};

const uint64_t MapMode = 0b11110000;
const uint64_t MapSize = 0b00001111;

template <typename T>
class __Config {

    friend class std::shared_ptr<__Config<T>>;

    NOT_COPIABLE(__Config)
    NOT_MOVABLE(__Config)
    NOT_COPY_ASSIGNABLE(__Config<T>)
    NOT_MOVE_ASSIGNABLE(__Config<T>)

#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) T m_ ## N = D;

#include "config.inc"

#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) QMap<T, QString> m_ ## N ## Options;

#include "config.inc"

#undef CONFIG_DEFINE

    const QString m_ConfigFilePath = CONFIG_FILE_PATH;

    QSet<QString> m_RestrictedVariables;
    QMap<QString, ServerVariablesPreset> m_Presets;
    QMap<QString, uint32_t> m_MapTags;

    static QMutex s_InitLock;
#define LOCK_GUARD QMutexLocker<QMutex> _guard(&s_InitLock)

    void AddOptions() {
#undef CONFIG_OPTIONS
#define CONFIG_OPTIONS(N, V, C, ...) m_ ## N ## Options.insert(V, C); \
        m_RestrictedVariables.insert(#N);

#include "config.inc"

#undef CONFIG_OPTIONS
    }

    void WriteConfig(bool init = false) const {
        LOCK_GUARD;

        QFile file(m_ConfigFilePath);
        file.open(QFile::OpenModeFlag::WriteOnly);

        QJsonObject configBody;
#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) MergeJsonObjects(configBody, {MAGIC(NEST(__VA_ARGS__ __VA_OPT__(,) #N, W(m_##N)))});

#undef CONFIG_SAVE_ONLY
#define CONFIG_SAVE_ONLY(T, N, D, W, ...) if (init) MergeJsonObjects(configBody, {MAGIC(NEST(__VA_ARGS__ __VA_OPT__(,) #N, W(D)))});

#include "config.inc"

#undef CONFIG_DEFINE
#undef CONFIG_SAVE_ONLY

        if (!init && configBody.contains("Presets")) {
            QJsonObject presets = configBody["Presets"].toObject();
            for (auto &it : GetPresetNames()) {
                // presets[it] = QJsonObject::fromVariantMap(GetPreset(it));
                MergeJsonObjects(configBody,
                    QJsonObject {{ "Presets", QJsonObject {{
                        it, QJsonObject::fromVariantMap(GetPreset(it)) }} }});
            }
        }

        file.write(QJsonDocument(configBody).toJson(QJsonDocument::Indented));
        file.close();
    }

    void ReadConfig(std::function<void(const QJsonObject &)> callback) {
        QFile config(m_ConfigFilePath);
        if (!config.exists()) {
            Log::AddLog(QString("[ERROR] Unable to open config file at \"%1\". Creating new one.").arg(m_ConfigFilePath));
            WriteConfig(true);
        }
        config.open(QFile::OpenModeFlag::ReadOnly);

        QJsonDocument configBody;

        {
            QByteArray data = config.readAll();
            QJsonParseError errorData;
            configBody = QJsonDocument::fromJson(data, &errorData);

            if (errorData.error != QJsonParseError::NoError || !configBody.isObject()) {
                Log::FatalLog(QString("[FATAL] Error on parsing config file: %1").arg(errorData.errorString()), errorData.error);
            }
        }

        LOCK_GUARD;

        int version = configBody.object()["ConfigVersion"].toInt(-1);
        switch (version) {
        case -1:
            Log::FatalLog("[FATAL] Config version invalid. Possible corruption of the config file.", -1);
            break;
        default:
            Log::AddLog(QString("[WARNING] Config version does not match current version. Current supported version: %1, config file version: %2. Continuing the program might cause unexpected issues.").arg(CONFIG_VERSION, version));
        case CONFIG_VERSION:
            callback(configBody.object());
            break;
        }

        config.close();
    }

    void PopulateConfigFields(const QJsonObject &config) {

// Fields
#undef ARG_ITER_DO
#define ARG_ITER_DO(X) [X].toObject()
#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) m_ ## N = W(config ARG_ITER(__VA_ARGS__) [#N]);
#undef GET_STR
#define GET_STR(X) X.toString()
#undef GET_INT
#define GET_INT(X) X.toInt()
#undef GET_QJSONARRAY
#define GET_QJSONARRAY(X) X.toArray()

// Map Tags
#undef MAP_TAG
#define MAP_TAG(MAP, C, B, Q, D, S, M, L, V) m_MapTags.insert(MAP, ( \
    (C * MapTags::Conquest) | \
    (B * MapTags::Breakthrough) | \
    (Q * MapTags::CloseQuarter) | \
    (D * MapTags::Destruction) | \
    (S * MapTags::Small) | \
    (M * MapTags::Medium) | \
    (L * MapTags::Large) | \
    (V * MapTags::VeryLarge) \
));

#include "config.inc"

#undef ARG_ITER_DO
#undef CONFIG_DEFINE
#undef GET_STR
#undef GET_INT
#undef GET_QJSONARRAY
#undef MAP_TAG

        const QJsonObject &presets = config["Presets"].toObject();
        for (const QString &key : presets.keys()) {
            if (key == "_CurrentPreset") continue;
            m_Presets.insert(key, presets[key].toObject().toVariantMap());
        }
    }

    void ReadServerConnectionSettings(const QJsonObject &config) {
#undef ARG_ITER_DO
#define ARG_ITER_DO(X) [X].toObject()
#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) m_ ## N = W(config ARG_ITER(__VA_ARGS__) [#N]);
#undef GET_STR
#define GET_STR(X) X.toString()
#undef GET_INT
#define GET_INT(X) X.toInt()

        CONFIG_DEFINE(QString, RconAddress, "127.0.0.1", GET_STR, "Rcon")
        CONFIG_DEFINE(QString, RconPassword, "example", GET_STR, "Rcon")
        CONFIG_DEFINE(unsigned short, RconPort, 27175, GET_INT, "Rcon")

#undef ARG_ITER_DO
#undef CONFIG_DEFINE
#undef GET_STR
#undef GET_INT
    }

    explicit __Config() {
        AddOptions();
        ReadConfig(std::bind(&__Config::PopulateConfigFields, this, std::placeholders::_1));
    }

public:

    static std::shared_ptr<__Config<T>> Get() {
        static std::shared_ptr<__Config<T>> s_Instance(new __Config());
        return s_Instance;
    }

    void UpdateConfig() const {
        WriteConfig();
    }

    void ReloadServerSettings() {
        ReadConfig(std::bind(&__Config::ReadServerConnectionSettings, this, std::placeholders::_1));
    }

#undef CONFIG_DEFINE
#define CONFIG_DEFINE(T, N, D, W, ...) \
    const T &Get ## N() const { return m_ ## N; } \
    void Set ## N(const T &value) { m_ ## N = value; } \
    QMap<T, QString> Get ## N ## Options() const { if (m_RestrictedVariables.contains(#N)) return {m_ ## N ## Options}; else return {}; }

#include "config.inc"

#undef CONFIG_DEFINE

    bool IsVariableRestricted(const QString &name) const { return m_RestrictedVariables.contains(name); }

    QList<QString> GetPresetNames() const { return m_Presets.keys(); }

    ServerVariablesPreset GetPreset(const QString &name) const {
        if (m_Presets.contains(name))
            return m_Presets[name];
        return {};
    }

    void SavePreset(const QString &name, const ServerVariablesPreset &preset) {
        if (m_Presets.contains(name))
            m_Presets.remove(name);
        m_Presets.insert(name, ServerVariablesPreset(preset));
    }

    QList<QString> FilterMapByTag(const uint32_t tags) const {
        QList<QString> result = m_MapTags.keys();
        erase_if(result, [this, &tags](QString x) {
            return !((m_MapTags[x] & tags & MapSize) && (m_MapTags[x] & tags & MapMode));
        });
        return result;
    }

    bool IsMapUsableInMode(const QString &map, const uint32_t tag) const {
        return m_MapTags[map] & tag & MapMode;
    }
};

template <typename T> QMutex __Config<T>::s_InitLock;

typedef __Config<int> Config;

#undef LOCK_GUARD

// =================================================================================================
//         CONTROLLER
// =================================================================================================

template <typename T>
class __Controller {

    std::shared_ptr<Config> m_Config;
    NetworkRunner *m_Runner;
    QThread m_Thread;

    __Controller(): m_Config(Config::Get()), m_Runner(new NetworkRunner()) {
        // Log::AddLog("Controller created");
        m_Runner->moveToThread(&m_Thread);
        QObject::connect(&m_Thread, SIGNAL(started()), this->m_Runner, SLOT(StartPolling()));
        m_Thread.setObjectName("Network Thread");
        m_Thread.start();
    }
public:
    static std::shared_ptr<__Controller> Get() {
        static std::shared_ptr<__Controller> s_Instance(new __Controller());
        return s_Instance;
    }

    void StopThread() {
        if (m_Thread.isRunning()) {
            m_Runner->Stop();
            m_Thread.wait();
        }
    }

    ~__Controller() {
        StopThread();
        m_Runner->deleteLater();
    }
};

typedef __Controller<int> Controller;

#endif // CONTROLLER_H
