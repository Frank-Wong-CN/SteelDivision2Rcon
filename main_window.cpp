#include "main_window.h"
#include "ui_main_window.h"

#include "src/controller.h"

#include <QApplication>
#include <QInputDialog>

#define Config Config::Get()

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QThread::currentThread()->setObjectName("Main Thread");
    ui->setupUi(this);
    m_CurrentPreviewMap = Config->GetMap();
    m_pMapScene = new QGraphicsScene();
    ui->vwMap->setScene(m_pMapScene);

    // Populate everything
    PopulateWidgets();

    // Connect models
    m_TblServerVariables = new ServerVariablesModel(this, ui->tblServerVariables, &m_ModServerVariables);
    m_TblPlayers = new ClientInfoModel(this, ui->tblCurrentPlayers, &m_ModPlayers);

    // Initialize UI state
    on_TabChanged(ui->tbwInfo->currentIndex());
    ui->tblServerVariables->horizontalHeader()->resizeSection(0, 80);
    ui->tblServerVariables->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tblServerVariables->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tblServerVariables->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    ui->tblBlacklist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblBlacklist->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    ui->tblCurrentPlayers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblCurrentPlayers->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    ui->listMap->setModel(&m_ListMaps);
    UpdateMapList();

    ui->listPreset->setModel(&m_ListPresets);
    m_ListPresets.setStringList(m_ModPresets);

    ui->lblAbout->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->lblAbout->setOpenExternalLinks(true);

    // Connect slots
    QObject::connect(ui->tbwInfo, SIGNAL(tabBarClicked(int)), this, SLOT(on_TabChanged(int)));
    QObject::connect(NotificationDelegate::Get().get(), SIGNAL(Broadcast(NotificationMessage)), this, SLOT(on_NotificationReceived(NotificationMessage)), Qt::QueuedConnection);
    QObject::connect(m_TblServerVariables, &ServerVariablesModel::dataChanged, this, [this](QModelIndex index, QModelIndex, QList<int> roles) {
        if (roles.empty() || roles[0] != Qt::EditRole) return;
        ServerVariableItemData &data = m_TblServerVariables->getServerVariableAtIndex(index);
        if (m_ServerVariableSetters.contains(data.m_Varname)) {
            if (data.m_Varname == "VictoryCond" && data.GetValue<INT_TYPE>() == 5) {
                // If chose Breakthrough, automatically change combat rule to Conquest
                ServerVariableItemData &combatRule = m_TblServerVariables->getServerVariable("CombatRule");
                combatRule.SetValue<INT_TYPE>(2);
                m_ServerVariableSetters["CombatRule"](2);
                m_TblServerVariables->updateView(2); // Update view
            }
            else if (data.m_Varname == "CombatRule" && data.GetValue<INT_TYPE>() == 1 && m_TblServerVariables->getServerVariable("VictoryCond").GetValue<INT_TYPE>() == 5) {
                // If chose Destruction and combat rule is Breakthrough, rollback changes and prompt a warning
                data.SetValue<INT_TYPE>(Config->GetCombatRule());
                QMessageBox(QMessageBox::Icon::Information, QObject::tr("Error"), QObject::tr("Destruction is not applicable to Breakthrough!"), QMessageBox::StandardButton::Ok, this).exec();
                m_TblServerVariables->updateView(2); // Update view
                return;
            }
            m_ServerVariableSetters[data.m_Varname](data.GetVariantValue());
        }
        else {
            Log::AddLog(QString("[ERROR] Variable \"%1\" does not exist.").arg(data.m_Varname));
        }
        UpdateMapList();
        // Update map preview
        if (Config->IsMapUsableInMode(m_CurrentPreviewMap, m_CurrentMapTag)) {
            PreviewMap(m_CurrentPreviewMap);
        }
        else {
            PreviewMap(m_AvailableMaps[0]);
        }
        SaveConfig();
    });
    QObject::connect(m_TblPlayers, &ClientInfoModel::setDeck, this, [this](const uint64_t clientID) {
        bool ok;
        QString text = QInputDialog::getText(this, QObject::tr("Set Deck"), QObject::tr("Deck Code: "), QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty()) {
            NotificationMessage msg = { 0, RconSetDeck };
            snprintf(msg.m_Buffer, 1000, "%llu PlayerDeckContent %s", clientID, text.toStdString().c_str());
            SEND_NOTIFY(msg);
        }
    });
    QObject::connect(m_TblPlayers, &ClientInfoModel::setDifficulty, this, [this](const uint64_t clientID, const uint32_t difficulty) {
        NotificationMessage msg = { 0, RconSetAIDifficulty };
        snprintf(msg.m_Buffer, 1000, "%llu PlayerIALevel %d", clientID, difficulty);
        SEND_NOTIFY(msg);
    });

    // Attempt auto connect
    if (Config->GetAutoConnect()) {
        ui->btnConnect->setEnabled(false);
        ui->btnConnect->setText(QObject::tr("Connecting"));
        SEND_NOTIFY({ 0, ConnectRconServer });
    }

    Log::AddLog("Ready.");
}

MainWindow::~MainWindow()
{
    SaveConfig();
    delete ui;
}

// =================================================================================================
//         METHODS
// =================================================================================================

#undef CONFIG_DEFINE
#undef CONFIG_CURRENT
#define CONFIG_DEFINE(...)
#define CONFIG_CURRENT(T, N, D, W) \
void SaveConfig ## N(QVariant value) { Config->Set ## N(value.value<T>()); }

#include "src/config.inc"

#undef CONFIG_DEFINE
#undef CONFIG_CURRENT

template <typename T>
QMap<QString, QVariant> ListTypeConverter(const QMap<T, QString> &list) {
    QMap<QString, QVariant> retval;
    for (auto &&i : list.keys()) {
        retval.insert(list[i], i);
    }
    return retval;
}

void MainWindow::PopulateWidgets() {

#undef CONFIG_DEFINE
#undef CONFIG_CURRENT
#define CONFIG_DEFINE(...)
#define CONFIG_CURRENT(T, N, D, W) \
    if (#N != "Map") { \
        m_ServerVariableSetters.insert(#N, SaveConfig ## N); \
        if (!Config->IsVariableRestricted(#N)) \
            m_ModServerVariables.push_back({ false, #N, QVariant::fromValue(Config->Get ## N()) }); \
        else \
            m_ModServerVariables.push_back({ false, #N, QVariant::fromValue(ValueOption({ Config->Get ## N(), ListTypeConverter<T>(Config->Get ## N ## Options()) })) }); \
    }

#include "src/config.inc"

#undef CONFIG_DEFINE
#undef CONFIG_CURRENT

    ui->lblServerNameContent->setText(Config->GetServerName());
    ui->lblServerPasswordContent->setText(Config->GetPassword());
    ui->lblServerPlayersContent->setText(QString("0/%1").arg(Config->GetNbMaxPlayer()));
    ui->lblMapContent->setText(QString("%1 (%2)").arg(Config->GetMap(), Config->GetMapOptions()[Config->GetMap()]));

    ui->edtAddress->setText(Config->GetRconAddress());
    ui->edtPassword->setText(Config->GetRconPassword());
    ui->spnPort->setValue(Config->GetRconPort());

    for (auto &&ref : Config->GetBannedPlayers()) {
        QJsonObject obj = ref.toObject();
        // TODO: Implement Blacklist management
    }

    for (auto &&ref : Config->GetPresetNames()) {
        if (ref == "_CurrentPreset") continue;
        m_ModPresets.push_back(ref);
    }
}

void MainWindow::SaveConfig() {
    // Server variables are saved immediately after editing, so no need for getting them from the UI
    Config->UpdateConfig();
}

void MainWindow::UpdateMapList() {

    m_CurrentMapTag = 0;

#define MapSizeChk(S) if (ui->chkMap ## S ->isChecked()) m_CurrentMapTag |= MapTags::S

    MapSizeChk(Small);
    MapSizeChk(Medium);
    MapSizeChk(Large);
    MapSizeChk(VeryLarge);

#undef MapSizeChk

    if (m_CurrentMapTag == 0) {
        m_CurrentMapTag = MapTags::Small |
                          MapTags::Medium |
                          MapTags::Large |
                          MapTags::VeryLarge;
    }

    int32_t vc, cr;
    vc = Config->GetVictoryCond();
    cr = Config->GetCombatRule();

#define GameMode(VC, CR) vc == VC && cr == CR

    if (GameMode(2, 2)) { // Conquest
        m_CurrentMapTag |= MapTags::Conquest;
        m_CurrentMapSuffix = "";
    }
    else if (GameMode(5, 2)) { // Breakthrough
        m_CurrentMapTag |= MapTags::Breakthrough;
        m_CurrentMapSuffix = "_BKT";
    }
    else if (GameMode(3, 2)) { // Close Quarter
        m_CurrentMapTag |= MapTags::CloseQuarter;
        m_CurrentMapSuffix = "_CQC";
    }
    else if (GameMode(2, 1)) { // Destruction
        m_CurrentMapTag |= MapTags::Destruction;
        m_CurrentMapSuffix = "_DEST";
    }
    else if (GameMode(3, 1)) { // Destruction Close Quarter
        m_CurrentMapTag |= MapTags::Destruction;
        m_CurrentMapSuffix = "_DEST_CQC";
    }

#undef GameMode

    QList<QString> list = Config->FilterMapByTag(m_CurrentMapTag);
    m_AvailableMaps = list;
    m_ListMaps.setStringList(list);

    emit m_ListMaps.dataChanged(m_ListMaps.index(0), m_ListMaps.index(m_ListMaps.rowCount()));
}

void MainWindow::UpdateMap(const QString &str) {
    RconUIState(false);
    Config->SetMap(str);
    ui->lblMapContent->setText(QString("%1 (%2)").arg(str, Config->GetMapOptions()[str]));
    SEND_NOTIFY({ 0, RconMapRequest, &m_CurrentMapSuffix });
}

void MainWindow::PreviewMap(const QString &str) {
    m_pMapScene->clear();
    m_pMapScene->addPixmap(QPixmap(QString("maps/%2/%1.png").arg(str, m_CurrentMapSuffix)));
    ui->vwMap->fitInView(m_pMapScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->vwMap->centerOn(m_pMapScene->itemsBoundingRect().center());
}

void MainWindow::ApplyPreset(const QString &str) {
    if (!Config->GetPresetNames().contains(str)) {
        QMessageBox(QMessageBox::Icon::Information, QObject::tr("Error"), QString("%1%2").arg(QObject::tr("Preset does not exist: "), str), QMessageBox::StandardButton::Ok, this).exec();
        return;
    }
    for (ServerVariableItemData &ref : m_ModServerVariables) ref.m_Selected = false;
    const auto &preset = Config->GetPreset(str);
    const auto &keys = preset.keys();
    for (auto &it : keys) {
        auto &item = m_TblServerVariables->getServerVariable(it);
        if (item.GetVarTypeId() == QMetaType::Int) {
            item.SetValue<INT_TYPE>(preset[it].toInt());
            m_ServerVariableSetters[it](preset[it].toInt());
        }
        else if (item.GetVarTypeId() == QMetaType::QString) {
            item.SetValue<STR_TYPE>(preset[it].toString());
            m_ServerVariableSetters[it](preset[it].toString());
        }
        else {
            Log::AddLog(QString("[ERROR] Unknown type ID when applying preset to variable \"%1\"!").arg(it));
            return;
        }
        item.m_Selected = true;
    }
    m_TblServerVariables->updateView(0); // Saves config
    m_TblServerVariables->updateView(2); // Saves config
    RconUIState(false);
    SEND_NOTIFY({ 0, RconRequest, &m_ModServerVariables });
}

void MainWindow::UpdatePreset() {
    const QString &name = ui->edtPresetName->text();
    if (name == "_CurrentPreset") {
        QMessageBox(QMessageBox::Icon::Warning, QObject::tr("Error"), QObject::tr("This preset name is reserved!"), QMessageBox::StandardButton::Ok, this).exec();
        return;
    }
    if (Config->GetPresetNames().contains(name)) {
        if (QMessageBox::Cancel == QMessageBox(QMessageBox::Icon::Question,
               QObject::tr("Warning"),
               QObject::tr("Preset with the same name already exists, overwrite?"),
               QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel,
               this).exec()) {
            return;
        }
    }
    ServerVariablesPreset preset;
    for (const ServerVariableItemData &ref : m_ModServerVariables) {
        if (!ref.m_Selected) continue;
        QVariant value;
        if (ref.m_Value.typeId() == qMetaTypeId<ValueOption>()) {
            const ValueOption valopt = ref.m_Value.value<ValueOption>();
            value = valopt.m_Value;
        }
        else {
            value = ref.m_Value;
        }
        preset.insert(ref.m_Varname, value);
    }
    Config->SavePreset(name, preset);
    SaveConfig();
    m_ListPresets.setStringList(Config->GetPresetNames());
    emit m_ListPresets.dataChanged(m_ListPresets.index(0), m_ListPresets.index(m_ListPresets.rowCount()));
}

void MainWindow::RconUIState(bool enabled) {
    ui->btnApplyServerVariables->setEnabled(enabled);
    ui->btnApplyPreset->setEnabled(enabled);
    ui->btnWaitPlayer->setEnabled(enabled);
    ui->btnReady->setEnabled(enabled);
    ui->btnChangeMap->setEnabled(enabled);
    ui->listMap->setEnabled(enabled);
}

// =================================================================================================
//         EVENTS
// =================================================================================================

void MainWindow::resizeEvent(QResizeEvent *event) {
    ui->vwMap->fitInView(m_pMapScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->vwMap->centerOn(m_pMapScene->itemsBoundingRect().center());
}

// =================================================================================================
//         SLOTS
// =================================================================================================

void MainWindow::on_TabChanged(int tab) {
    if (tab == m_MapTabIndex) {
        ui->vwMap->show();
        PreviewMap(m_CurrentPreviewMap);
        ui->layoutQuickButtonsWidget->hide();
    }
    else {
        ui->vwMap->hide();
        ui->layoutQuickButtonsWidget->show();
    }
}

void MainWindow::on_NotificationReceived(NotificationMessage msg) {
    switch (msg.m_MessageType) {
    case LogMessage: {
        while (Log::HasLog()) {
            QMutexLocker _guard(&m_Logging);
            QString log = QString("[%1] %2").arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), Log::PopLog());
            ui->textLog->appendPlainText(log);
            ui->statusBar->showMessage(log);
        }
        break;
    }
    case ConnectedRconServer: {
        ui->btnConnect->setText(QObject::tr("Connected"));
        ui->btnConnect->setEnabled(true);
        ui->tbwInfo->setCurrentIndex(0);
        Config->SetAutoConnect(1);
        SaveConfig();
        if (Config->GetAutoSyncVariablesOnConnect()) {
            SEND_NOTIFY({ 0, RconSyncVariables, &m_ModServerVariables });
        }
        break;
    }
    case DisconnectedRconServer: {
        ui->btnConnect->setText(QObject::tr("Connect"));
        ui->btnConnect->setEnabled(true);
        Config->SetAutoConnect(0);
        SaveConfig();
        break;
    }
    case RconRequestDone: {
        RconUIState(true);
        bool checkMap = false;
        for (ServerVariableItemData &ref : m_ModServerVariables) {
            if (ref.m_Selected && (ref.m_Varname == "CombatRule" || ref.m_Varname == "VictoryCond")) {
                checkMap = true;
            }
            ref.m_Selected = false;
        }
        if (checkMap) {
            // Update map variant
            if (Config->IsMapUsableInMode(Config->GetMap(), m_CurrentMapTag)) {
                UpdateMap(Config->GetMap());
            }
            else {
                Log::AddLog("Current map not usable in current mode. Reverting to the first map in the map list.");
                UpdateMap(m_AvailableMaps[0]);
            }
        }
        m_TblServerVariables->updateView(0); // Saves config
        break;
    }
    case RconPlayerRequestDone: {
        QMap<uint64_t, QString> *clients = (QMap<uint64_t, QString> *)msg.m_Sender;

        ui->lblServerPlayersContent->setText(QString(std::format("{}/{}", clients->size(), Config->GetNbMaxPlayer()).c_str()));
        m_TblPlayers->addClient(clients);

        break;
    }
    default:
        break;
    }
}

// =================================================================================================
//         SLOTS (CONNECT BY NAME)
// =================================================================================================

void MainWindow::on_btnSaveServerSettings_clicked() { SaveConfig(); }

void MainWindow::on_btnLoadServerSettings_clicked()
{
    Config->ReloadServerSettings();

    ui->edtAddress->setText(Config->GetRconAddress());
    ui->edtPassword->setText(Config->GetRconPassword());
    ui->spnPort->setValue(Config->GetRconPort());
}

void MainWindow::on_edtAddress_textChanged(const QString &arg1) { Config->SetRconAddress(arg1); }
void MainWindow::on_edtPassword_textChanged(const QString &arg1) { Config->SetRconPassword(arg1); }
void MainWindow::on_spnPort_valueChanged(int arg1) { Config->SetRconPort(arg1); }

void MainWindow::on_btnConnect_clicked() {
    ui->btnConnect->setEnabled(false);
    ui->btnConnect->setText(QObject::tr("Connecting"));
    SEND_NOTIFY({ 0, ConnectRconServer });
}

void MainWindow::on_btnApplyServerVariables_clicked() {
    RconUIState(false);
    SEND_NOTIFY({ 0, RconRequest, &m_ModServerVariables });
}

void MainWindow::on_chkMapSmall_stateChanged(int arg1)  { UpdateMapList(); }
void MainWindow::on_chkMapMedium_stateChanged(int arg1)  { UpdateMapList(); }
void MainWindow::on_chkMapLarge_stateChanged(int arg1) { UpdateMapList(); }
void MainWindow::on_chkMapVeryLarge_stateChanged(int arg1)  { UpdateMapList(); }

void MainWindow::on_btnChangeMap_clicked() {
    UpdateMap(m_ListMaps.data(ui->listMap->currentIndex()).toString());
}

void MainWindow::on_listMap_doubleClicked(const QModelIndex &index) {
    UpdateMap(m_ListMaps.data(index).toString());
}

void MainWindow::on_listMap_clicked(const QModelIndex &index) {
    m_CurrentPreviewMap = m_ListMaps.data(index).toString();
    PreviewMap(m_CurrentPreviewMap);
}

void MainWindow::on_listPreset_clicked(const QModelIndex &index) {
    QString text;
    const QString &presetName = m_ListPresets.data(index).toString();
    const auto &preset = Config->GetPreset(presetName);
    const auto &keys = preset.keys();
    for (auto &it : keys) {
        text.append(QString("%1 = %2\n").arg(it, preset[it].toString()));
    }
    ui->textPresetPreview->setPlainText(text);
    ui->edtPresetName->setText(presetName);
}

void MainWindow::on_listPreset_doubleClicked(const QModelIndex &index) {
    ApplyPreset(m_ListPresets.data(index).toString());
}

void MainWindow::on_btnApplyPreset_clicked() {
    ApplyPreset(m_ListPresets.data(ui->listPreset->currentIndex()).toString());
}

void MainWindow::on_btnUpdatePreset_clicked() {
    UpdatePreset();
}

void MainWindow::on_btnWaitPlayer_clicked() {
    ApplyPreset("WaitPlayers");
    // TODO: Kick all computers
}

void MainWindow::on_btnReady_clicked() {
    ApplyPreset("ReadyToLaunch");
}

void MainWindow::on_btnApplyAll_clicked() {
    for (auto &it : m_ModServerVariables) {
        if (it.m_Varname == "ServerName" ||
            it.m_Varname == "ServerPassword")
            it.m_Selected = false;
        it.m_Selected = true;
    }
    RconUIState(false);
    SEND_NOTIFY({ 0, RconRequest, &m_ModServerVariables });
}

void MainWindow::on_btnSetDiffAll_clicked() {
    bool ok;
    QString text = QInputDialog::getItem(this, QObject::tr("Set Difficulty"), QObject::tr("Choose difficulty: "), ClientInfoItemData::s_Strings, 0, false, &ok);
    uint32_t diff = ClientInfoItemData::s_Strings.indexOf(text);
    if (!ok || diff == 0) return;
    for (auto &it : m_ModPlayers) {
        if (it.IsComputer()) {
            NotificationMessage msg = { 0, RconSetAIDifficulty };
            snprintf(msg.m_Buffer, 1000, "%llu PlayerIALevel %d", it.m_ClientID, diff);
            SEND_NOTIFY(msg);
        }
    }
}

void MainWindow::on_btnKick_clicked() {
    //
}

