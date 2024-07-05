#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QGraphicsScene>
#include <QMutex>

#include "src/notification_delegate.h"
#include "src/server_variables_model.h"
#include "src/view_models.h"

#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ServerVariablesModel *m_TblServerVariables;
    ServerVariables m_ModServerVariables;
    ClientInfoModel *m_TblPlayers;
    ClientList m_ModPlayers;
    // m_TblBlacklist;

    QStringListModel m_ListMaps;
    QStringListModel m_ListPresets;
    QList<QString> m_AvailableMaps;
    QList<QString> m_ModPresets;

    QMap<QString, std::function<void(QVariant)>> m_ServerVariableSetters;

    QString m_CurrentPreviewMap;
    QString m_CurrentMapSuffix;
    uint64_t m_CurrentMapTag;

    QGraphicsScene *m_pMapScene;

    QMutex m_Logging;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_TabChanged(int tab);
    void on_NotificationReceived(NotificationMessage msg);

    void on_btnSaveServerSettings_clicked();
    void on_btnLoadServerSettings_clicked();
    void on_btnConnect_clicked();

    void on_edtAddress_textChanged(const QString &arg1);
    void on_edtPassword_textChanged(const QString &arg1);
    void on_spnPort_valueChanged(int arg1);

    void on_btnApplyServerVariables_clicked();

    void on_chkMapSmall_stateChanged(int arg1);
    void on_chkMapMedium_stateChanged(int arg1);
    void on_chkMapLarge_stateChanged(int arg1);
    void on_chkMapVeryLarge_stateChanged(int arg1);

    void on_btnChangeMap_clicked();
    void on_listMap_doubleClicked(const QModelIndex &index);
    void on_listMap_clicked(const QModelIndex &index);

    void on_listPreset_clicked(const QModelIndex &index);
    void on_listPreset_doubleClicked(const QModelIndex &index);
    void on_btnApplyPreset_clicked();
    void on_btnUpdatePreset_clicked();
    void on_btnWaitPlayer_clicked();
    void on_btnReady_clicked();

    void on_btnApplyAll_clicked();
    void on_btnSetDiffAll_clicked();

    void on_btnKick_clicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    const int m_MapTabIndex = 2;

    void PopulateWidgets();
    void SaveConfig();
    void UpdateMapList();
    void UpdateMap(const QString &str);
    void PreviewMap(const QString &str);

    void ApplyPreset(const QString &str);
    void UpdatePreset();

    void RconUIState(bool enabled);
};
#endif // MAIN_WINDOW_H
