#ifndef VIEW_MODELS_H
#define VIEW_MODELS_H

#include <QAbstractTableModel>
#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>
#include <QObject>
#include <QTableView>
#include <QComboBox>
#include <QPushButton>

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#include <QString>
#include <QDateTime>

const uint64_t ComputerIDPrefix = ((uint64_t)1 << 56) | ((uint64_t)1 << 57); // 216172782113783808

struct BlacklistItemData {
    uint64_t m_ClientID;
    QDateTime m_BannedUntil;
};

class BlacklistModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BlacklistModel(QObject *parent = nullptr);
};

struct ClientInfoItemData {
    uint64_t m_ClientID;
    QString m_ClientNickname;
    uint32_t m_Difficulty;

    QString DifficultyString() const;
    inline bool IsComputer() { return m_ClientID & ComputerIDPrefix; }

    static const QVector<QString> s_Strings;
};

typedef QList<ClientInfoItemData> ClientList;

class ClientInfoDifficultyDropdownDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ClientInfoDifficultyDropdownDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class ClientInfoSetDeckDelegate : public QAbstractItemDelegate {
    Q_OBJECT
public:
    explicit ClientInfoSetDeckDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void buttonClicked(uint32_t row);
};

class ClientInfoModel : public QAbstractTableModel {
    Q_OBJECT

    friend class ClientInfoDifficultyDropdownDelegate;

    ClientInfoDifficultyDropdownDelegate *m_pDropdownDelegate;
    ClientInfoSetDeckDelegate *m_pSetDeckDelegate;
    ClientList *m_pClientList;
    QTableView *m_pView;

    bool clientExists(uint64_t id);

public:
    explicit ClientInfoModel(QObject *parent, QTableView *tableView, ClientList *model);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addClient(const QMap<uint64_t, QString> *rawClientList);
    void updateView();

signals:
    void setDeck(const uint64_t clientID);
    void setDifficulty(const uint64_t clientID, const uint32_t difficulty);
};

#endif // VIEW_MODELS_H
